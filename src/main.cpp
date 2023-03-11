#include <sched.h>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "include/cpufreq.h"
#include "include/log.h"
#include "include/frame_analyze.h"
#include "include/jank_analyze.h"
#include "include/close_others.h"
#include "include/misc.h"

using std::cout;
using namespace std::chrono;
using namespace std::this_thread;
using namespace std::filesystem;

int main()
{
    bound2little();
    cout.sync_with_stdio(false);
    std::cout << std::unitbuf;

    Log &log = Log::getLog("/storage/emulated/0/Android/FAS/FasLog.txt");
    log.setLevel(LogLevel::Info);
    log.setLevel(LogLevel::Debug);
    log.write(LogLevel::Info, "Log started");

    Cpufreq &cpu_controller = Cpufreq::getCpufreq();
    log.write(LogLevel::Info, "Creating virtual frequency:");

    for (const auto &i : cpu_controller.get_super_table())
        log.write(LogLevel::Info, ("Virtual Freq: " + std::to_string(i)).c_str());
    log.write(LogLevel::Info, "Successfully created virtual frequency table");

    cpu_controller.set_scaling(2);

    start_close_others();
    log.write(LogLevel::Info, "Starting cleanup process");

    while (true)
    {
        while (getSurfaceview().find(getTopApp()) == string::npos && !getTopApp().empty())
        {
            sleep_for(1s);
            log.write(LogLevel::Debug, "Not game");
            cpu_controller.limit_clear();
        }

        sleep_for(100ms);

        const jank_data jdata = analyzeFrameData(getOriginalData());
        if (jdata.empty())
        {
            log.write(LogLevel::Debug, "Empty jank data!");
            continue;
        }

        /* nice是超时帧占所有帧的百分率 */
        log.write(LogLevel::Debug, std::to_string(jdata.nice()).c_str());

        const double nice = jdata.nice();
        constexpr double left = 0.005;
        constexpr double right = 0.01;
        if (nice >= left && nice <= right)
            log.write(LogLevel::Debug, "Proportion of frame delay is in line with expectation");
        else if (nice < left) // 掉帧少了，有余量
        {
            cpu_controller.limit(-1);
            log.write(LogLevel::Debug, "Proportion of frame delay is less than expectation");
        }
        else
        {
            log.write(LogLevel::Debug, "Proportion of frame delay exceeded expectation");
            if (nice <= right * 11 / 10) // 掉帧多了，卡顿
            {
                log.write(LogLevel::Debug, "Exceeded expectation rating: 1");
                cpu_controller.limit(1);
            }
            else if (nice <= right * 12 / 10)
            {
                log.write(LogLevel::Debug, "Exceeded expectation rating: 2");
                cpu_controller.limit(2);
            }
            else
            {
                log.write(LogLevel::Debug, "Exceeded expectation rating: 3");
                cpu_controller.limit(3);
            }
        }
    }
}
