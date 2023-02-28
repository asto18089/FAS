#include <cmath>
#include <sched.h>
#include <iostream>
#include <chrono>
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

    CPU_SET(0, &mask);
    CPU_SET(1, &mask);
    CPU_SET(2, &mask);
    CPU_SET(3, &mask);

    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
}

int main()
{
    bound2little();
    cout.sync_with_stdio(false);

    Cpufreq cpu_controller;
    cpu_controller.set_scaling(2);

    start_close_others();

    while (true)
    {
        while (getSurfaceview().empty())
        {
            sleep_for(seconds(1));
            cpu_controller.limit_clear();
        }

        const jank_data jdata = analyzeFrameData(getOriginalData());

        sleep_for(milliseconds(100));

        /* nice是超时帧占所有帧的百分率 */

        // cout << jdata.nice() << endl;

        if (jdata.nice() >= 0.02 && jdata.nice() <= 0.05) // 掉帧刚刚好
            continue;
        else if (jdata.nice() <= 0.04) // 掉帧少了，有余量
            cpu_controller.limit(-1);
        else if (jdata.nice() <= 0.1) // 掉帧多了，卡顿
            cpu_controller.limit(1);
        else
            cpu_controller.limit(std::pow(jdata.nice() * 10, 2));
    }
}
