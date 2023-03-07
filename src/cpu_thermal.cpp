#include <iostream>
#include <thread>
#include <sys/prctl.h>
#include <filesystem>
#include <array>
#include <chrono>

using std::string;
using std::array;
using namespace std::filesystem;
using namespace std::this_thread;
using namespace std::chrono;

Cputhermal::Cputhermal()
{
    const array<const char*> TYPENAMES = {"soc_max", "soc_top", "cpu_big", "soc", "cpu"};
    
    auto node_finder = [&](const char* type)
    {
        for (const auto& entry : directory_iterator("/sys/devices/virtual/thermal"))
        {
            if (! entry.is_directory())
                continue;       
                
            const path& type_file = entry.path() / "type";
            if (exists(type_file) && is_regular_file(type_file) && status(type_file).permissions() & perms::owner_read)
            {
                std::ifstream ifs(type_file);
                string content;
                std::getline(ifs, content);
                if (ifs.find(type))
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
    
    this->SuperFreqTable = makeSuperFreqTable(freqdiff);
    inline_freq = *SuperFreqTable.cbegin()
    
    std::thread tempmonitor(Cputhermal::temp_monitor, temp_node, std::ref(this->temp));
    tempmonitor.detach();
}

void Cputhermal::temp_monitor(string temp_node, int& temp)
{
    prctl(PR_SET_NAME, "TempWatcher");
    
    std::ifstream temp_fd;
    while (true)
    {
        sleep_for(millionseconds(50));
        temp_fd.open(temp_node);
        temp_fd >> temp;
        temp_fd.close();
        
        if ()
        
        
    }
}
