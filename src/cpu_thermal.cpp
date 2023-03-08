#include <iostream>
#include <thread>
#include <sys/prctl.h>
#include <filesystem>
#include <array>
#include <chrono>

#include "include/lockvalue.h"
#include "include/cpu_thermal.h"

using std::string;
using std::array;
using std::vector;
using namespace std::filesystem;
using namespace std::this_thread;
using namespace std::chrono;

Cputhermal::Cputhermal()
{
    const array<const char*, 5> TYPENAMES = {"soc_max", "soc_top", "cpu_big", "soc", "cpu"};
    
    auto node_finder = [&](const char* type)
    {
        for (const auto& entry : directory_iterator("/sys/devices/virtual/thermal"))
        {
            if (! entry.is_directory())
                continue;       
                
            const path& type_file = entry.path() / "type";
            if (exists(type_file) && is_regular_file(type_file))
            {
                std::ifstream ifs(type_file);
                string content;
                std::getline(ifs, content);
                if (content.find(type))
                {
                    temp_node = entry.path() / "temp";
                    return true;
                }
            }
        }
        return false;
    };
    
    for (const auto& i : TYPENAMES)
    {
        if (node_finder(i))
            break;
    }
    
    // 读取该集群的最大和最小频率
    auto readMAM = [](const string& policyname)
    {
        unsigned long max(0), min(0);
        std::ifstream fd;
        
        fd.open("/sys/devices/system/cpu/cpufreq/" + policyname + "/cpuinfo_max_freq");
        fd >> max;
        fd.close();
        
        fd.open("/sys/devices/system/cpu/cpufreq/" + policyname + "/cpuinfo_min_freq");
        fd >> min;
        fd.close();
        
        return std::pair{max, min};
    };
    
    unsigned long maxfreq(0), minfreq(std::numeric_limits<unsigned long>::max());
 
    for (const auto& entry : directory_iterator("/sys/devices/system/cpu/cpufreq"))
    {
        const string& policyname = entry.path().filename();
        const auto& mam = readMAM(policyname);
        
        maxfreq = std::max(maxfreq, mam.first);
        minfreq = std::min(minfreq, mam.second);
    }
    
    // 创建超级频率表(用于控制所有集群)
    auto makeSuperFreqTable = [&](const unsigned long& freqdiff)
    {
        vector<unsigned long> freqtable;
        
        for (unsigned long freq = maxfreq; freq >= minfreq; freq -= freqdiff)
            freqtable.push_back(freq);

        return freqtable; // 返回向量作为结果
    };
    
    this->SuperFreqTable = makeSuperFreqTable(50000);
    
    std::thread temp_policy(Cputhermal::temp_policy);
    temp_policy.detach();
}

void Cputhermal::temp_policy()
{
    prctl(PR_SET_NAME, "TempWatcher");
    
    Cputhermal& thermal = Cputhermal::getCputhermal();
    std::ifstream temp_fd;
    while (true)
    {
        sleep_for(milliseconds(50));
        
        temp_fd.open(thermal.temp_node);
        temp_fd >> thermal.temp;
        
        if (thermal.temp >= thermal.target_temp && thermal.kpi < thermal.SuperFreqTable.size())
            thermal.kpi++;
        else if (thermal.temp < thermal.target_temp)
            thermal.kpi = 0;
        
        thermal.inline_freq = thermal.SuperFreqTable[thermal.kpi];
        
        for (const auto &entry : directory_iterator("/sys/devices/system/cpu/cpufreq"))
        {
            const auto& freq = thermal.inline_freq > thermal.target_freq ? thermal.target_freq : thermal.inline_freq;
            Lockvalue(entry.path().string() + "/scaling_max_freq", freq);
        }
        
        temp_fd.close();
    }
}

void Cputhermal::set_target_freq(const unsigned long& freq)
{
    Cputhermal& thermal = Cputhermal::getCputhermal();
    thermal.target_freq = freq;
}

void Cputhermal::set_target_temp(const int& temp)
{
    Cputhermal& thermal = Cputhermal::getCputhermal();
    thermal.target_temp = temp;
}