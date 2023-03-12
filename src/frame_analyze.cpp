#include <cstdio>
#include <charconv>
#include <array>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <sys/prctl.h>
#include <iostream>

#include "include/frame_analyze.h"
#include "include/misc.h"
#include "include/log.h"

using namespace std::chrono;

string getSurfaceview()
{
    static string last;
    string game = execCmdSync("/system/bin/dumpsys", {"SurfaceFlinger", "--list"});
    std::istringstream ss_game(game);
    string buf;
    while (std::getline(ss_game, buf))
    {
        auto clear = [&]()
        {
            if (!last.empty() && last != buf)
                execCmdSync("/system/bin/dumpsys", {"SurfaceFlinger", "--latency-clear"});
            last = buf;
        };
        if (buf.find("SurfaceView[") != string::npos && buf.find("BLAST") != string::npos)
        {
            clear();
            return buf;
        }
        if (buf.find("SurfaceView -") != string::npos)
        {
            clear();
            return buf;
        }
    }
    return {};
}

void FtimeStamps::getOriginalData()
{
    DEBUG("Start dumping frame time data");

    string dumpsys = execCmdSync("/system/bin/dumpsys", {"SurfaceFlinger", "--latency", getSurfaceview()});

    if (dumpsys.empty())
        return;

    std::istringstream iss(dumpsys);
    static string analyze, analyze_last;
    string analyze_last_t;
    
    while (std::getline(iss, analyze))
    {
        std::array<unsigned long, 3> timestamps = {0};
        
        if (analyze_last == analyze && !analyze_last.empty())
        {
            start_timestamps.clear();
            vsync_timestamps.clear();
            end_timestamps.clear();

            analyze_last.clear();
            continue;
        }
        
        for (size_t pos = 0, i = 0; pos < analyze.length();)
        {
            pos = std::find_if_not(analyze.cbegin() + pos, analyze.cend(), [](char c)
                                   { return !std::isdigit(c); }) - analyze.cbegin();

            if (pos == analyze.length())
                break;

            const size_t end = std::find_if_not(analyze.cbegin() + pos + 1, analyze.cend(), [](char c)
                                          { return std::isdigit(c); }) - analyze.cbegin();

            if (i < timestamps.size())
                std::from_chars(analyze.data() + pos, analyze.data() + end, timestamps[i]);

            pos = end;
            i++;
        }
        
        // 等于 0 或大于等于 10000000000000000
        auto pred = [](const auto &i) { return i == 0 || i >= 10000000000000000; };
        auto it = std::find_if(timestamps.cbegin(), timestamps.cend(), pred);

        if (it != timestamps.cend())
            continue;
        else
        {
            start_timestamps.push_back(timestamps[0]);
            vsync_timestamps.push_back(timestamps[1]);
            end_timestamps.push_back(timestamps[2]);
        }
        
        analyze_last_t = analyze;
    }
    
    analyze_last = std::move(analyze_last_t);
}

void FtimeStamps::fpsWatcher(int &fps)
{
    prctl(PR_SET_NAME, "FpsWatcher");
    auto time_A = steady_clock::now();
    auto time_B = steady_clock::now();
    string dump_A, dump_B;
    while (true)
    {
        auto parall_stamp_dump = [](steady_clock::time_point& time_T, string& dump_T)
        {
            std::thread stamp([&]() {time_T = steady_clock::now();});
            std::thread dump([&]() {dump_T = execCmdSync("/system/bin/service", {"call", "SurfaceFlinger", "1013"});});
            stamp.join();
            dump.join();
        };

        parall_stamp_dump(time_A, dump_A);
        dump_A = dump_A.substr(15, 8);
        int frame_A = stoi(dump_A, nullptr, 16);
        
        sleep_for(1s);

        parall_stamp_dump(time_B, dump_B);
        dump_B = dump_B.substr(15, 8);
        int frame_B = stoi(dump_B, nullptr, 16);

        fps = frame_B - frame_A / (duration_cast<microseconds>(time_B - time_A).count() / 1000 / 1000);
        DEBUG("Fps :" + std::to_string(fps));
    }
}