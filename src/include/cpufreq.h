#pragma once

#include <vector>
using std::vector;

class Cpufreq
{
    vector<unsigned long> SuperFreqTable;
    int kpi = 0;
    int scaling = 2;
    void makeFreqTable(const unsigned long);
    void writeFreq();
    Cpufreq();
public:
    static Cpufreq& getCpufreq() {
        static Cpufreq instance;
        return instance;
    }
    Cpufreq(const Cpufreq &) = delete; // dont copy this
    vector<unsigned long> get_super_table();
    void limit(const int);
    void limit_clear();
    void set_scaling(const int);
};
