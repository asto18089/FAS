#include <iostream>
#include <thread>
#include <sys/prctl.h>
#include <filesystem>
#include <array>
#include <chrono>

#include "include/lockvalue.h"
#include "include/cpu_thermal.h"

using std::array;
using std::string;
using std::string_view;
using std::vector;
using namespace std::filesystem;
using namespace std::this_thread;
using namespace std::chrono;

Cputhermal::Cputhermal()
{
    constexpr array<string_view, 5> TYPENAMES = {"soc_max", "soc_top", "cpu_big", "soc", "cpu"};
    auto node_finder = [&](const string_view &type)
    {
        for (const auto &entry : directory_iterator("/sys/devices/virtual/thermal"))
        {
            if (!entry.is_directory())
                continue;

            const path &type_file = entry.path() / "type";
            if (exists(type_file) && is_regular_file(type_file))
            {
                std::ifstream ifs(type_file);
                string content;
                ifs >> content;
                if (content.find(type) != string::npos)
                {
                    temp_node = entry.path() / "temp";
                    return true;
                }
            }
        }
        return false;
    };

    for (const string_view &i : TYPENAMES)
    {
        if (node_finder(i))
            break;
    }

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

    unsigned long maxfreq(0), minfreq(std::numeric_limits<unsigned long>::max());

    for (const auto &entry : directory_iterator("/sys/devices/system/cpu/cpufreq"))
    {
        const string policyname = entry.path().filename();
        const auto mam = readMAM(policyname);

        maxfreq = std::max(maxfreq, mam.first);
        minfreq = std::min(minfreq, mam.second);
    }

    // 创建超级频率表(用于控制所有集群)
    auto makeSuperFreqTable = [&](const unsigned long &freqdiff)
    {
        vector<unsigned long> freqtable;

        for (unsigned long freq = maxfreq; freq >= minfreq; freq -= freqdiff)
            freqtable.push_back(freq);

        return freqtable; // 返回数组作为结果
    };

    this->SuperFreqTable = makeSuperFreqTable(50000);

    std::thread temp_policy(Cputhermal::temp_policy);
    temp_policy.detach();
}

void Cputhermal::temp_policy()
{
    prctl(PR_SET_NAME, "TempWatcher");

    Cputhermal &thermal = Cputhermal::getCputhermal();
    std::ifstream temp_fd;
    int temp = 0;
    while (true)
    {
        sleep_for(10ms);

        temp_fd.open(thermal.temp_node);
        temp_fd >> temp;
        thermal.temp = temp / 1000;

        if (thermal.temp >= thermal.target_temp && thermal.kpi < thermal.SuperFreqTable.size() - 1)
            thermal.kpi++;
        else if (thermal.temp < thermal.target_temp && thermal.kpi > 0)
            thermal.kpi--;

        thermal.inline_freq = thermal.SuperFreqTable[thermal.kpi];
        temp_fd.close();

        for (const auto &entry : directory_iterator("/sys/devices/system/cpu/cpufreq"))
        {
            const string location = entry.path().string() + "/scaling_max_freq";

            if (thermal.policy_freq.contains(location))
                Lockvalue(location, std::min(thermal.policy_freq[location], thermal.inline_freq));
            else
                Lockvalue(location, thermal.inline_freq);
        }
    }
}

void Cputhermal::TLockvalue(const string &location, const unsigned long &freq)
{
    Cputhermal &thermal = Cputhermal::getCputhermal();
    thermal.policy_freq[location] = freq;
}

void Cputhermal::set_target_temp(const int &temp)
{
    Cputhermal &thermal = Cputhermal::getCputhermal();
    thermal.target_temp = temp;
}
