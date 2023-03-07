#pragma once

#include <fstream>
#include <string>
#include <vector>

class Cputhermal
{
    std::vector<unsigned long> SuperFreqTable;
    int cpu_temp = 0;
    int target_temp = 75;
    unsigned long inline_freq = 0;
    std::string temp_node;
    static void temp_monitor(std::string, int&);
    Cputhermal();
public:
    static Cputhermal& getCputhermal() {
        static Cputhermal instance;
        return instance;
    }
    void thermal_freqwriter(const unsigned long&);
    void set_target_temp(const int&);
};