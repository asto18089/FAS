#pragma once

#include <fstream>

class Cputhermal
{
    int cpu_temp = 0;
    std::ifstream temp_node;
    static void temp_monitor(int&);
public:
    Cputhermal();
    ~Cputhermal();
    void thermal_freqwriter(const unsigned long&);
};