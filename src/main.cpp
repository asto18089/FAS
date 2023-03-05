#include <sched.h>
#include <iostream>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <thread>

#include "include/cpufreq.h"
#include "include/log.h"
#include "include/frame_analyze.h"
#include "include/jank_analyze.h"
#include "include/close_others.h"

using std::cout;
using namespace std::chrono;
using namespace std::this_thread;
using namespace std::filesystem;

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
    std::cout << std::unitbuf;

    Log log = Log(current_path().string + "/log.txt");
    log.setLevel(LogLevel::Info);

    Cpufreq cpu_controller;
    log.write(LogLevel::Info, "Creating virtual frequency:")
    for (const auto& i : cpu_controller.get_super_table())
        log.write(LogLevel::Info, "Virtual Freq: " + std::to_string(i));
    log.write(LogLevel::Info, "The virtual frequency table was created successfully");
    
    cpu_controller.set_scaling(2);

    start_close_others();
    log.write(LogLevel::Info, "The cleanup process starts")

    auto cost = steady_clock::now();
    int speedup = 0;
    
    while (true)
    {
        while (getSurfaceview().empty())
        {
            sleep_for(seconds(1));
            cpu_controller.limit_clear();
        }
        
        auto sleep_dynamic = [&](const int& ms)
        {
            const auto& realtime = milliseconds(ms) - duration_cast<milliseconds>(steady_clock::now() - cost);
            if (realtime > milliseconds(speedup))
                sleep_for(realtime - milliseconds(speedup));
            else
                sleep_for(realtime);
        };
        sleep_dynamic(125);
        
        const jank_data jdata = analyzeFrameData(getOriginalData());
        if (jdata.empty())
        {
            speedup = -50;
            continue;
        }

        /* nice是超时帧占所有帧的百分率 */

        cout << jdata.nice() << '\n';
        auto niceFreq = [&](const double& left, const double& right)
        {
            const double& nice = jdata.nice();
            if (nice >= left && nice <= right) // 掉帧刚刚好
            {
                speedup = -20;
            }
            else if (nice < left) // 掉帧少了，有余量
            {
                speedup = 5;
                cpu_controller.limit(-1);
            }
            else if (nice <= right * 11 / 10) // 掉帧多了，卡顿
            {
                speedup = 10;
                cpu_controller.limit(3);
            }
            else if (nice <= right * 12 / 10)
            {
                speedup = -10;
                cpu_controller.limit(4);
            }
            else
            {
                speedup = -10;
                cpu_controller.limit(5);
            }
        };
        niceFreq(0.01, 0.015);
            
        cost = steady_clock::now();
    }
}
