#pragma once

#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

using std::vector;
using std::ofstream;
using std::ios;

// Edit and Lock a file
template <typename T>
bool Lockvalue(const char* location, T value)
{
    chmod(location, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH);
    ofstream fd(location, ios::out | ios::trunc);
    if (!fd)
    {
        fd.close();
        return false;
    }
    fd << value;
    fd.close();
    chmod(location, S_IRUSR | S_IRGRP | S_IROTH);
    return true;
}

class Cpufreq {
    vector<unsigned long> middle_cpu_table;
    vector<unsigned long> big_cpu_table;
    int kpi;
    int kpi_min;
    void getFreq();
    void Cpu_big_limit();
    void Cpu_middle_limit();
    void start_cpu_writer();
    static void cpu_writer(Cpufreq& device);
public:
    Cpufreq();
    Cpufreq(const Cpufreq &other) = delete; //dont copy this
    void show_middle_table();
    void show_big_table();
    void limit(const int& n);
    void limit_clear();
};
