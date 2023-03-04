#pragma once

#include <map>
#include <vector>

using std::vector;
using std::map;

class Cpufreq
{
    map<int, vector<unsigned long>> freqmap;
    int kpi = 0;
    int kpi_min = 0;
    int scaling = 2;
    void getFreq();
    void Cpu_big_limit();
    void Cpu_middle_limit();

public:
    Cpufreq();
    Cpufreq(const Cpufreq &) = delete; // dont copy this
    void show_middle_table();
    void show_big_table();
    void limit(const int &);
    void limit_clear();
    void set_scaling(const int &);
};
