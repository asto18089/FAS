#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <sys/prctl.h>
#include <chrono>

#include "include/frame_analyze.h"

using std::cout;
using std::endl;
using std::ref;
using std::string;
using std::thread;
using std::vector;
using namespace std::chrono;
using namespace std::this_thread;

string getSurfaceview()
{
    FILE *game = popen("dumpsys SurfaceFlinger --list", "r");

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
            return result;
        } // 安卓11以及以上用的方法

        if (result.find("SurfaceView -") != string::npos)
        {
            result.pop_back();
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

    string cmd = "dumpsys SurfaceFlinger --latency \'" + getSurfaceview() + "\'";
    FILE *dumpsys = popen(cmd.c_str(), "r");

    char buffer[1024] = {0};
    static string analyze_last;

    while (fgets(buffer, sizeof(buffer), dumpsys))
    {
        static string analyze;
        static unsigned long timestamps[3] = {0};
        analyze = buffer;
        analyze.pop_back();

        if (analyze_last == analyze)
        {
            Fdata.start_time_stamps.clear();
            Fdata.vsync_time_stamps.clear();
            Fdata.end_time_stamps.clear();
            continue;
        }

        static bool found = false;
        for (size_t pos = 0, len = 0, i = 0, pos_b = 0; pos < analyze.length(); pos++)
        {
            const bool isnumber = (analyze[pos] <= '9' && analyze[pos] >= '0');

            if (!found && isnumber)
            {
                pos_b = pos;
                found = true;
            }
            else if (found && (!isnumber || pos == analyze.length() - 1))
            {
                len = pos - pos_b + 1;
                found = false;

                timestamps[i] = atol(analyze.substr(pos_b, len).c_str());
                i++;
            }
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
            else if (i >= 99999999999999)
                goto ANALYZE_END;
        }

        Fdata.start_time_stamps.push_back(timestamps[0]);
        Fdata.vsync_time_stamps.push_back(timestamps[1]);
        Fdata.end_time_stamps.push_back(timestamps[2]);

        analyze_last = analyze;

    ANALYZE_END:
        continue;
    }

    return Fdata;
}
