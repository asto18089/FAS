#include <cmath>
#include <sched.h>
#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <thread>

#include "include/cpufreq.h"
#include "include/frame_analyze.h"
#include "include/jank_analyze.h"
#include "include/close_others.h"

using std::cout;
using namespace std::chrono;
using namespace std::this_thread;

static void bound2little()
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    
    std::ifstream fd("/sys/devices/system/cpu/cpufreq/policy0/related_cpus");
    string related_cpus;
    std::getline(fd, related_cpus);
    std::istringstream cut(related_cpus);
    
    short cpu = 0;
    while (cut >> cpu)
    {
        CPU_SET(cpu, &mask);
    }

    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
}

int main()
{
    bound2little();
    cout.sync_with_stdio(false);

    Cpufreq cpu_controller;
    cpu_controller.show_super_table();
    cpu_controller.set_scaling(2);

    start_close_others();

    auto cost = steady_clock::now();
    int speedup = 0;
    
    while (true)
    {
        while (getSurfaceview().empty())
        {
            sleep_for(seconds(1));
            cpu_controller.limit_clear();
        }
        
        sleep_for(milliseconds(125) - duration_cast<milliseconds>(steady_clock::now() - cost) - milliseconds(speedup));
        
        const jank_data jdata = analyzeFrameData(getOriginalData());
        if (jdata.empty())
        {
            speedup = -50;
            continue;
        }

        /* nice是超时帧占所有帧的百分率 */

        // cout << jdata.nice() << endl;

        if (jdata.nice() >= 0.02 && jdata.nice() <= 0.05) // 掉帧刚刚好
        {
            speedup = -20;
        }
        else if (jdata.nice() <= 0.04) // 掉帧少了，有余量
        {
            speedup = 5;
            cpu_controller.limit(-1);
        }
        else if (jdata.nice() <= 0.1) // 掉帧多了，卡顿
        {
            speedup = 10;
            cpu_controller.limit(1);
        }
        else
        {
            speedup = -10;
            cpu_controller.limit(std::pow(jdata.nice() * 10, 2));
        }
            
        cost = steady_clock::now();
    }
}
