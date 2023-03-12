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

    INFO("Log started");

    Cpufreq &cpu_controller = Cpufreq::getCpufreq();
    INFO("Creating virtual frequency:");

    for (const auto &i : cpu_controller.get_super_table())
        INFO("Virtual Freq: " + std::to_string(i));
    INFO("Successfully created virtual frequency table");

    cpu_controller.set_scaling(2);

    start_close_others();
    INFO("Starting cleanup process");

    while (true)
    {
        while (getSurfaceview().find(getTopApp()) == string::npos || getTopApp().empty() || getSurfaceview().empty())
        {
            sleep_for(500ms);
            cpu_controller.limit_clear();
        }

        const jank_data jdata = analyzeFrameData(getOriginalData());
        if (jdata.empty())
            continue;

        /* nice是超时帧占所有帧的百分率 */
        DEBUG(std::to_string(jdata.nice()));

        const double nice = jdata.nice();
        constexpr double left = 0.003;
        constexpr double right = 0.005;
        if (nice >= left && nice <= right)
            DEBUG("Proportion of frame delay is in line with expectation");
        else if (nice < left) // 掉帧少了，有余量
        {
            cpu_controller.limit(-1);
            DEBUG("Proportion of frame delay is less than expectation");
        }
        else
        {
            DEBUG("Proportion of frame delay exceeded expectation");
            if (nice <= right * 13 / 10) // 掉帧多了，卡顿
            {
                DEBUG("Exceeded expectation rating: 1");
                cpu_controller.limit(1);
            }
            else if (nice <= right * 15 / 10)
            {
                DEBUG("Exceeded expectation rating: 2");
                cpu_controller.limit(3);
            }
            else
            {
                DEBUG("Exceeded expectation rating: 3");
                cpu_controller.limit(5);
            }
        }
    }
}
