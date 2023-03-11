#include <cstdio>
#include <charconv>
#include <array>
#include <algorithm>
#include <chrono>

#include "include/frame_analyze.h"
#include "include/log.h"

Log &log_frame = Log::getLog("/storage/emulated/0/Android/FAS/FasLog.txt");

using namespace std::chrono;

string getSurfaceview()
{
    log_frame.write(LogLevel::Debug, "Start dumping Surfaceview");
    static string result;
    static auto stamp = steady_clock::now();
    if (duration_cast<milliseconds>(steady_clock::now() - stamp) < 1s && !result.empty())
        return result;
    FILE *game = popen("dumpsys SurfaceFlinger --list 2>/dev/null", "r");

    char buffer[1024] = {0};

    if (game == nullptr)
    {
        perror("Failed");
        pclose(game);

        return {}; // It's empty
    }

    while (fgets(buffer, sizeof(buffer), game))
    {
        result = buffer;
        if ((result.find("SurfaceView[") != string::npos && result.find("BLAST") != string::npos) || // 安卓11以及以上用的方法
             result.find("SurfaceView -") != string::npos)                                           // 安卓11以下的方法
        {
            result.pop_back();
            break;
        }
        /*安卓9以下的方法还不一样，不过没有必要适配*/
        result.clear();
    }

    pclose(game);
    stamp = steady_clock::now();
    log_frame.write(LogLevel::Debug, "Dumped Surfaceview");
    return result;
}

FtimeStamps getOriginalData()
{
    log_frame.write(LogLevel::Debug, "Start dumping frame time data");
    FtimeStamps Fdata;
    const string cmd = "dumpsys SurfaceFlinger --latency \'" + getSurfaceview() + "\' 2>/dev/null";
    FILE *dumpsys = popen(cmd.c_str(), "r");

    if (dumpsys == nullptr)
    {
        perror("Failed");
        pclose(dumpsys);

        return Fdata;
    }

    char buffer[1024] = {0};
    static string analyze, analyze_last;
    string analyze_last_t;

    while (std::fgets(buffer, sizeof(buffer), dumpsys))
    {
        static std::array<unsigned long, 3> timestamps = {0};
        analyze = buffer;
        analyze.pop_back();

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
    pclose(dumpsys);
    log_frame.write(LogLevel::Debug, "Dumped Frame time data");
    return Fdata;
}
