#include <string>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "include/cpufreq.h"
#include "include/lockvalue.h"

using std::string;
using std::vector;
using namespace std::filesystem;

Cpufreq::Cpufreq()
{
    makeFreqTable(100000);
    limit_clear();
}

void Cpufreq::makeFreqTable(const unsigned long& freqdiff) // 建议freqdiff为100mhz
{
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
    
    for (const auto& entry : directory_iterator("/sys/devices/system/cpu/cpufreq/"))
    {
        const auto& policyname = entry.path().filename();
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
    
    SuperFreqTable = makeSuperFreqTable(freqdiff);
}

void Cpufreq::show_super_table()
{
    for (const auto i : SuperFreqTable)
        std::cout << i;
    std::cout << '\n';
}

void Cpufreq::writeFreq()
{
    directory_iterator freqdir = directory_iterator("/sys/devices/system/cpu/cpufreq/");
    directory_entry end_entry;
    for (const auto& entry : freqdir) // 保存最后一个entry
        end_entry = entry;
        
    for (const auto& entry : freqdir)
        entry != end_entry ? Lockvalue(entry.path().string() + "/scaling_max_freq", SuperFreqTable[kpi - scaling]) : Lockvalue(entry.path().string() + "/scaling_max_freq", SuperFreqTable[kpi]);
}

void Cpufreq::limit(const int& change)
{
    if (kpi + change >= 0 && kpi < 0)
        kpi = kpi + change;
    else
        kpi = 0;
    if (kpi + change <= SuperFreqTable.size() && change > 0)
        kpi = kpi + change;
    else
        kpi = SuperFreqTable.size();
        
    writeFreq();
}

void Cpufreq::limit_clear()
{
    kpi = 0;
    writeFreq();
}

void Cpufreq::set_scaling(const int& new_scaling)
{
    scaling = new_scaling;
}