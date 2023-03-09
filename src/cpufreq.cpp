#include <string>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "include/cpufreq.h"
#include "include/cpu_thermal.h"

using std::string;
using std::vector;
using namespace std::filesystem;

Cpufreq::Cpufreq()
{
    makeFreqTable(50000);
    limit_clear();
}

void Cpufreq::makeFreqTable(const unsigned long freqdiff) // 建议freqdiff为50mhz
{
    // 读取该集群的最大和最小频率
    auto readMAM = [](const string &policyname)
    {
        unsigned long max = 0, min = 0;
        std::ifstream fd;

        fd.open("/sys/devices/system/cpu/cpufreq/" + policyname + "/cpuinfo_max_freq");
        fd >> max;
        fd.close();

        fd.open("/sys/devices/system/cpu/cpufreq/" + policyname + "/cpuinfo_min_freq");
        fd >> min;
        fd.close();

        return std::make_pair(max, min);
    };

    unsigned long maxfreq = 0, minfreq = std::numeric_limits<unsigned long>::max();

    for (const auto &entry : directory_iterator("/sys/devices/system/cpu/cpufreq"))
    {
        const auto policyname = entry.path().filename();
        if (policyname == "policy0")
            continue;
        const auto mam = readMAM(policyname);

        maxfreq = std::max(maxfreq, mam.first);
        minfreq = std::min(minfreq, mam.second);
    }

    // 创建超级频率表(用于控制所有集群)
    auto makeSuperFreqTable = [&](const unsigned long freqdiff)
    {
        vector<unsigned long> freqtable;

        for (unsigned long freq = maxfreq; freq >= minfreq; freq -= freqdiff)
            freqtable.push_back(freq);

        return freqtable; // 返回向量作为结果
    };

    SuperFreqTable = makeSuperFreqTable(freqdiff);
    writeFreq(); // 让温控启动
}

vector<unsigned long> Cpufreq::get_super_table()
{
    return SuperFreqTable;
}

void Cpufreq::writeFreq()
{
    directory_entry end_entry;
    for (const auto &entry : directory_iterator("/sys/devices/system/cpu/cpufreq")) // 保存最后一个entry
        end_entry = entry;

    for (const auto &entry : directory_iterator("/sys/devices/system/cpu/cpufreq"))
    {
        if (entry.path().filename() == "policy0")
            continue;

        if (entry != end_entry)
        {
            if (kpi + scaling <= SuperFreqTable.size() - 1)
                Cputhermal::TLockvalue(entry.path().string() + "/scaling_max_freq", SuperFreqTable[kpi + scaling]);
            else
                Cputhermal::TLockvalue(entry.path().string() + "/scaling_max_freq", *SuperFreqTable.cend() - 1);
        }
        else
            Cputhermal::TLockvalue(entry.path().string() + "/scaling_max_freq", SuperFreqTable[kpi]);
    }
    /*std::cout << kpi << ' ';
    std::cout << SuperFreqTable[kpi] << '\n';*/
}

void Cpufreq::limit(const int change_in)
{
    int change;
    
    if (change_in > 0 && kpi <= SuperFreqTable.size() * 6 / 10)
        change = -change_in * 2 / 3;
    else if (change_in > 0 && kpi <= SuperFreqTable.size() * 8 / 10)
        change = -change_in / 2;
        
    if (change_in < 0 && kpi <= SuperFreqTable.size() * 6 / 10)
        change = -change_in * 3 / 2;
    else if (change_in < 0 && kpi <= SuperFreqTable.size() * 8 / 10)
        change = -change_in * 2;
 
    if (change < 0)
    {
        if (kpi + change >= 0)
            kpi += change;
        else
            kpi = 0;
    }
    else
    {
        if (kpi + change <= SuperFreqTable.size() - 1)
            kpi += change;
        else
            kpi = SuperFreqTable.size() - 1;
    }

    writeFreq();
}

void Cpufreq::limit_clear()
{
    kpi = 0;
    auto readM = [](const string &policyname)
    {
        unsigned long max = 0;
        std::ifstream fd;

        fd.open("/sys/devices/system/cpu/cpufreq/" + policyname + "/cpuinfo_max_freq");
        fd >> max;
        return max;
    };

    for (const auto &entry : directory_iterator("/sys/devices/system/cpu/cpufreq"))
    {
        const string policyname = entry.path().filename();
        Cputhermal::TLockvalue("/sys/devices/system/cpu/cpufreq/" + policyname + "/scaling_max_freq", readM(policyname));
    }
}

void Cpufreq::set_scaling(const int new_scaling)
{
    scaling = new_scaling;
}
