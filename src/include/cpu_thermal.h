#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class Cputhermal
{
    std::vector<unsigned long> SuperFreqTable;
    int temp = 0;
    int target_temp = 75;
    int kpi = 0;
    unsigned long inline_freq = 0;
    std::unordered_map<std::string, unsigned long> policy_freq;
    std::string temp_node;
    Cputhermal();

public:
    static Cputhermal &getCputhermal()
    {
        static Cputhermal instance = Cputhermal();
        return instance;
    }
    static void temp_policy();
    static void TLockvalue(const std::string &, const unsigned long);
    static void node_target_temp();
};
