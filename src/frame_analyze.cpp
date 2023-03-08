#include <cstdio>
#include <charconv>
#include <array>
#include <algorithm>

#include "include/frame_analyze.h"

string getSurfaceview()
{
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
        string result = buffer;
        if ((result.find("SurfaceView[") != string::npos && result.find("BLAST") != string::npos) || // 安卓11以及以上用的方法
             result.find("SurfaceView -") != string::npos) // 安卓11以下的方法
        {
            result.pop_back();
            pclose(game);
            return result;
        }
        /*安卓9以下的方法还不一样，不过没有必要适配*/
    }

    pclose(game);
    return {};
}

FtimeStamps getOriginalData()
{
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

        analyze_last = analyze;
    }
    pclose(dumpsys);
    return Fdata;
}
