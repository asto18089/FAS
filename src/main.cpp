#include <sched.h>
#include <iostream>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

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

    int cpu = 0;
    while (cut >> cpu)
        CPU_SET(cpu, &mask);

    sched_setaffinity(0, sizeof(mask), &mask);
}

static string getTopapp()
{
    auto Testfile = [](const char *location) { return (access(location, F_OK) == 0); };
    
    string result;

    if (Testfile("/sys/kernel/gbe/gbe2_fg_pid"))
    {
        string pid;
        static std::ifstream f_pid, app;
        f_pid.open("/sys/kernel/gbe/gbe2_fg_pid");
        if (! f_pid)
            return {};
        f_pid >> pid;
        f_pid.close();

        app.open("/proc/" + pid + "/cmdline");
        if (! app)
            return {};
        app >> result;
        app.close();

        while (*(result.cend() - 1) == '\0')
            result.pop_back();
        return result;
    }

    FILE *app = popen(R"(dumpsys activity activities|grep topResumedActivity=|tail -n 1|cut -d "{" -f2|cut -d "/" -f1|cut -d " " -f3)", "r");

    char buffer[1024] = {0};

    if (app == nullptr)
    {
        perror("Failed");
        pclose(app);

        return {}; // It's empty
    }

    fgets(buffer, sizeof(buffer), app);
    result = buffer;
    result.pop_back();

    pclose(app);
    return result;
}

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
    log.write(LogLevel::Info, "The virtual frequency table was created successfully");

    cpu_controller.set_scaling(2);

    start_close_others();
    log.write(LogLevel::Info, "The cleanup process starts");

    while (true)
    {
        while (getSurfaceview().find(getTopapp()) == string::npos && ! getTopapp().empty())
        {
            sleep_for(1s);
            log.write(LogLevel::Debug, "Not game");
            cpu_controller.limit_clear();
        }

        sleep_for(100ms);

        const jank_data jdata = analyzeFrameData(getOriginalData());
        if (jdata.empty())
        {
            log.write(LogLevel::Debug, "Empty jank data !");
            continue;
        }

        /* nice是超时帧占所有帧的百分率 */
        log.write(LogLevel::Debug, std::to_string(jdata.nice()).c_str());

        const double nice = jdata.nice();
        constexpr double left = 0.01;
        constexpr double right = 0.02;
        if (nice >= left && nice <= right)
            log.write(LogLevel::Debug, "The proportion of frame delay is in line with expectations");
        else if (nice < left) // 掉帧少了，有余量
        {
            cpu_controller.limit(-1);
            log.write(LogLevel::Debug, "The proportion of frame delay is less than expectations");
        }
        else
        {
            log.write(LogLevel::Debug, "The proportion of frame delay is higher than expected");
            if (nice <= right * 11 / 10) // 掉帧多了，卡顿
            {
                log.write(LogLevel::Debug, "Exceeded Expectation Rating : 1");
                cpu_controller.limit(1);
            }
            else if (nice <= right * 12 / 10)
            {
                log.write(LogLevel::Debug, "Exceeded Expectation Rating : 2");
                cpu_controller.limit(2);
            }
            else
            {
                log.write(LogLevel::Debug, "Exceeded Expectation Rating : 3");
                cpu_controller.limit(3);
            }
        }
    }
}
