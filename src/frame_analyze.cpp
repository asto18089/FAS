#include <cstdio>
#include <charconv>
#include <array>
#include <algorithm>
#include <chrono>
#include <sstream>
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

FtimeStamps getOriginalData()
{
    DEBUG("Start dumping frame time data");
    FtimeStamps Fdata;

    string dumpsys = execCmdSync("/system/bin/dumpsys", {"SurfaceFlinger", "--latency", getSurfaceview()});
    
    if (dumpsys.empty())
        return Fdata;

    std::istringstream iss(dumpsys);
    static string analyze, analyze_last;
    string analyze_last_t;
    
    while (std::getline(iss, analyze))
    {
        std::array<unsigned long, 3> timestamps = {0};
        
        if (analyze_last == analyze && !analyze_last.empty())
        {
            Fdata.start_timestamps.clear();
            Fdata.vsync_timestamps.clear();
            Fdata.end_timestamps.clear();

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
            Fdata.start_timestamps.push_back(timestamps[0]);
            Fdata.vsync_timestamps.push_back(timestamps[1]);
            Fdata.end_timestamps.push_back(timestamps[2]);
        }
        
        analyze_last_t = analyze;
    }
    
    analyze_last = std::move(analyze_last_t);
    Fdata.fps = (int)((*(Fdata.vsync_timestamps.cend()--) - *Fdata.vsync_timestamps.cbegin()) / 1000 / 1000 / 1000 / 1000 / 10000) / Fdata.end_timestamps.size();
    std::cout << Fdata.fps << '\n';
    return Fdata;
}
