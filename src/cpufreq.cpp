#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <thread>
#include <sys/prctl.h>
#include <chrono>
#include <stdlib.h>
#include <string>

#include "include/cpufreq.h"

using std::vector;
using std::cout;
using std::endl;
using std::thread;
using std::ifstream;
using std::ref;
using std::string;

Cpufreq::Cpufreq() {
    kpi = 0;
    scaling = 2;
    getFreq();
}

void Cpufreq::getFreq() { //读取频率表
    ifstream list;
    string freq;
    unsigned short pos = 0;

    list.open("/sys/devices/system/cpu/cpufreq/policy4/scaling_available_frequencies"); //cpu4-6
    getline(list, freq);
    list.close();
    
    if(! list) {
        list.open("/sys/devices/system/cpu/cpufreq/policy3/scaling_available_frequencies"); 
        getline(list, freq);
        list.close();
    }
    
    freq.pop_back();
    for (auto i = 0; i < freq.length() - 1; i++) {
        if (freq[i] == ' ') {
            middle_cpu_table.push_back(atoi(freq.substr(pos, i - pos).c_str()));
            pos = i + 1;
        }
    }
    

    list.open("/sys/devices/system/cpu/cpufreq/policy7/scaling_available_frequencies"); //cpu7
    getline(list, freq);
    list.close();
    
    freq.pop_back();
    pos = 0;
    for (auto i = 0; i < freq.length() - 1; i++) {
        if (freq[i] == ' ') {
            big_cpu_table.push_back(atoi(freq.substr(pos, i - pos).c_str()));
            pos = i + 1;
        }
    }
    
    //频率从大到小
    sort(middle_cpu_table.begin(), middle_cpu_table.end(), std::greater<unsigned long>());
    sort(big_cpu_table.begin(), big_cpu_table.end(), std::greater<unsigned long>());
    
    //处理频率偏移算法
    for (kpi_min = 0; kpi_min < std::min(big_cpu_table.size(), middle_cpu_table.size()); kpi_min++) {
        if (big_cpu_table[kpi_min + 1] < middle_cpu_table[0]) {
            break;
        }
    }
    kpi_min = -1 * kpi_min;
}


void Cpufreq::show_middle_table() {
    for (auto i : middle_cpu_table) {
        cout << i << ' ';
    };  cout << endl;
}

void Cpufreq::show_big_table() {
    for (auto i : big_cpu_table) {
        cout << i << ' ';
    };  cout << endl;
}

void Cpufreq::Cpu_big_limit() {
    static int tmp(666), target(999);
    
    if (kpi - kpi_min - scaling < 0) {
        target = 0;
    }
    else if (kpi - kpi_min - scaling <= big_cpu_table.size()) {
        target = kpi - kpi_min - scaling;
    } else {
        target = big_cpu_table.size();
    }
    
    if (tmp != target) {
        Lockvalue("/sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq", big_cpu_table[target]);
        
        Lockvalue("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", big_cpu_table[target]);
    }
    tmp = target;
    //cout << "大核taget："<< *target <<  "kpi:" << kpi << endl;
}

void Cpufreq::Cpu_middle_limit() {
    static int tmp(666), target(999);
    
    if (kpi < 0) {
        target = 0;
    } else if (kpi <= middle_cpu_table.size()) {
        target = kpi;
    } else {
        target = middle_cpu_table.size();
    }
    
    if (tmp != target) {
        Lockvalue("/sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq", middle_cpu_table[target]);
        Lockvalue("/sys/devices/system/cpu/cpufreq/policy3/scaling_max_freq", middle_cpu_table[target]);
    }
    tmp = target;
    //cout << "中核taget："<< *target <<  "kpi:" << kpi << endl;
}

void Cpufreq::cpu_writer(Cpufreq& device) {
    prctl(PR_SET_NAME, "Cpufreq writer");
    
    device.Cpu_middle_limit();
    device.Cpu_big_limit();
}

void Cpufreq::limit_clear() {
    Lockvalue("/sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq", big_cpu_table[0]);  
    Lockvalue("/sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq", middle_cpu_table[0]);
    Lockvalue("/sys/devices/system/cpu/cpufreq/policy3/scaling_max_freq", middle_cpu_table[0]);
}

void Cpufreq::limit(const int& n) {
    if(n < 0) {
        if (kpi - n <= std::min(big_cpu_table.size(), middle_cpu_table.size())) {
            kpi = kpi - n;
        } else {
            kpi = std::min(big_cpu_table.size(), middle_cpu_table.size());
        }
    } else if (n > 0) {
        if (kpi - n >= kpi_min) {
            kpi = kpi - n;
        } else {
            kpi = kpi_min;
        }
    }
    
    thread t_cpu_writer(cpu_writer, ref(*this));
    t_cpu_writer.detach();
}