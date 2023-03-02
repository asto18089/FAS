#include <cstdio>
#include <charconv>
#include <string_view>
#include <array>

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
        if (result.find("SurfaceView[") != string::npos && result.find("BLAST") != string::npos)
        {
            result.pop_back();
            pclose(game);
            
            return result;
        } // 安卓11以及以上用的方法

        if (result.find("SurfaceView -") != string::npos)
        {
            result.pop_back();
            pclose(game);
            
            return result;
        } // 安卓11以下的方法

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
    static std::string_view analyze;
    static string analyze_last, analyze_last_t;

    while (std::fgets(buffer, sizeof(buffer), dumpsys))
    {
        static std::array<unsigned long, 3> timestamps = {0};
        analyze = buffer;

        if (analyze_last == analyze && ! analyze_last.empty())
        {
            Fdata.start_time_stamps.clear();
            Fdata.vsync_time_stamps.clear();
            Fdata.end_time_stamps.clear();
            
            analyze_last = {};
            continue;
        }

        for (size_t pos = 0, i = 0; pos < analyze.length();)
        {
            pos = std::find_if_not(analyze.begin() + pos, analyze.end(), [](char c) { return !std::isdigit(c); }) - analyze.begin();
            
            if (pos == analyze.length())
                break;

            size_t end = std::find_if_not(analyze.begin() + pos + 1, analyze.end(), [](char c) { return std::isdigit(c); }) - analyze.begin();
    
            if (i < timestamps.size())
                std::from_chars(analyze.data() + pos, analyze.data() + end, timestamps[i]);
            
            pos = end;
            i++;
        }

        for (const auto &i : timestamps)
        {
            if (i == 0)
            {
                Fdata.start_time_stamps.clear();
                Fdata.vsync_time_stamps.clear();
                Fdata.end_time_stamps.clear();
                goto ANALYZE_END;
            }
            else if (i >= 10000000000000000)
                goto ANALYZE_END;
        }

        Fdata.start_time_stamps.push_back(timestamps[0]);
        Fdata.vsync_time_stamps.push_back(timestamps[1]);
        Fdata.end_time_stamps.push_back(timestamps[2]);
        
        analyze_last_t = analyze;
        
    ANALYZE_END:
        continue;
    }
    
    analyze_last = std::move(analyze_last_t);
    
    pclose(dumpsys);
    return Fdata;
}
