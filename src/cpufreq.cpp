#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <stringview>
#include <filesystem>
#include <charconv>

#include "include/cpufreq.h"
#include "include/lockvalue.h"

using std::cout;
using std::endl;
using std::string;

using namespace std::filesystem;

Cpufreq::Cpufreq()
{
    getFreq();
    middle_cpu_table.reserve(20);
    big_cpu_table.reserve(20);
}

void Cpufreq::getFreq()
{
    map<size_t, vector<unsigned long>> freqtables;

    // 定义一个辅助函数来读取频率表并排序
    auto readAndSortFreq = [](const std::string_view& filename)
    {
        std::ifstream list;
        string freq;
        vector<unsigned long> table;

        list.open(filename);
        std::getline(list, freq);
        list.close();

        std::istringstream iss(freq);
        while (iss >> freq)
        {
            table.push_back(std::stol(freq));
        }
        // 频率从大到小
        std::sort(table.begin(), table.end(), std::greater<>());

        return table;
    };

    size_t policy_it = 0;
    for (const auto& entry : directory_iterator("/sys/devices/system/cpu/cpufreq/")) // 读取频率表
    {
        const auto& policyname = entry.path().filename();

        if (policyname == "policy0") // 忽略小核
            continue;
        
        freqtables[policy_it] = readAndSortFreq(entry.path().string() + "/scaling_available_frequencies");
        policy_it++;
    }

    size_t map_size = 0;
    for (const auto& [policy, table] : freqtables)
    {
        (table.size() > map_size) && (map_size = table.size());
    }

    // 获取最小的最大频率
    unsigned long min_maxFreq = 0;
    for (const auto& [key, table] : freqtables)
        *table.cbegin() < freq_temp && (min_maxFreq = *table.cbegin());

    // 获取该频率表距指定频率最近的频率的下标
    auto kpi_closest = [&](const vector<unsigned long>& v)
    {
        for (const auto& i = v.cbegin(); i < v.cend() - 1; i++)
        {
            if (*(i + 1) <= min_maxFreq)
                return (size_t)(i - v.cbegin());
        }

        return (size_t)v.size();
    };
    
    vector<size_t> kpi_l;
    for (const auto& [policy, table] : freqtables)
        kpi_l.push_back(kpi_closest(table));

    size_t& kpi_l_max = (std::max_element(kpi_l.cbegin(), kpi_l.cend()) - kpi_l.cbegin());
    
    for (size_t i = 0; i < map_size; i++)
    {
        for (size_t it = 0; i < freqtable.size(); i++)
        {
            
        }
    }
}

void Cpufreq::getFreq()
{
    // 定义一个辅助函数来读取频率表并排序
    auto readAndSortFreq = [](const std::string& filename, std::vector<unsigned long>& table)
    {
        std::ifstream list;
        std::string freq;

        list.open(filename);
        std::getline(list, freq);
        list.close();

        std::istringstream iss(freq);
        while (iss >> freq)
        {
            table.push_back(std::stol(freq));
        }
        // 频率从大到小
        std::sort(table.begin(), table.end(), std::greater<>());
    };
    
    readAndSortFreq("/sys/devices/system/cpu/cpufreq/policy4/scaling_available_frequencies", middle_cpu_table);
    if (middle_cpu_table.empty()) {
        readAndSortFreq("/sys/devices/system/cpu/cpufreq/policy3/scaling_available_frequencies", middle_cpu_table);
    }
    
    readAndSortFreq("/sys/devices/system/cpu/cpufreq/policy7/scaling_available_frequencies", big_cpu_table);

    // 处理频率偏移
    for (kpi_min = 0; kpi_min < std::min(big_cpu_table.size(), middle_cpu_table.size()); kpi_min++)
    {
        if (big_cpu_table[kpi_min + 1] < middle_cpu_table[0])
            break;
    }
    kpi_min = -kpi_min;
}

void Cpufreq::show_middle_table()
{
    for (const auto &i : middle_cpu_table)
        cout << i << ' ';
    cout << '\n';
}

void Cpufreq::show_big_table()
{
    for (const auto &i : big_cpu_table)
        cout << i << ' ';
    cout << '\n';
}

void Cpufreq::Cpu_big_limit()
{
    static int tmp = 666, target = 999;

    if (kpi - kpi_min - scaling < 0)
        target = 0;
    else if (kpi - kpi_min - scaling <= big_cpu_table.size())
        target = kpi - kpi_min - scaling;
    else
        target = big_cpu_table.size();

    if (tmp != target)
    {
        Lockvalue("/sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq", big_cpu_table[target]);
        Lockvalue("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", big_cpu_table[target]);
    }
    tmp = target;
    // cout << "大核target："<< *target <<  "kpi:" << kpi << endl;
}

void Cpufreq::Cpu_middle_limit()
{
    static size_t tmp = 666, target = 999;

    if (kpi < 0)
        target = 0;
    else if (kpi <= middle_cpu_table.size())
        target = kpi;
    else
        target = middle_cpu_table.size();

    if (tmp != target)
    {
        Lockvalue("/sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq", middle_cpu_table[target]);
        Lockvalue("/sys/devices/system/cpu/cpufreq/policy3/scaling_max_freq", middle_cpu_table[target]);
    }
    tmp = target;
    // cout << "中核target：" << *target <<  "kpi:" << kpi << endl;
}

void Cpufreq::limit_clear()
{
    Lockvalue("/sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq", *big_cpu_table.cbegin());
    Lockvalue("/sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq", *middle_cpu_table.cbegin());
    Lockvalue("/sys/devices/system/cpu/cpufreq/policy3/scaling_max_freq", *middle_cpu_table.cbegin());
}

void Cpufreq::limit(const int &n)
{
    if (n < 0)
    {
        if (kpi - n <= std::min(big_cpu_table.size(), middle_cpu_table.size()))
            kpi = kpi - n;
        else
            kpi = std::min(big_cpu_table.size(), middle_cpu_table.size());
    }
    else if (n > 0)
    {
        if (kpi - n >= kpi_min)
            kpi = kpi - n;
        else
            kpi = kpi_min;
    }
    Cpu_middle_limit();
    Cpu_big_limit();
}

void Cpufreq::set_scaling(const int &n)
{
    scaling = n;
}
