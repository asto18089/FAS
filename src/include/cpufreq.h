#pragma once

#include <vector>
#include <sys/stat.h>
#include <fstream>

using std::ios;
using std::ofstream;
using std::vector;

class Cpufreq
{
    vector<unsigned long> middle_cpu_table;
    vector<unsigned long> big_cpu_table;
    int kpi = 0;
    int kpi_min;
    short scaling = 2;
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
    void set_scaling(const short &);
};
