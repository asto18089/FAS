#include <vector>
#include <iostream>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

jank_data analyzeFrameData(const FtimeStamps &Fdata)
{
    jank_data Jdata;

    if (Fdata.vsync_time_stamps.size() < 4)
    {
        Jdata.big_jank_count = 66;
        Jdata.jank_count = 66;
        return Jdata;
    }

    // const unsigned long MOVIE_FRAME_TIME = 1000 * 1000 * 1000 / 24;

    const auto &vsync_begin = Fdata.vsync_time_stamps.begin();
    const auto &vsync_end = Fdata.vsync_time_stamps.end();

    /*for (auto i : Fdata.vsync_time_stamps) {
        cout << i << endl;
    }*/

    vector<unsigned long> vsysc_frametime;
    static unsigned long first_3_avg_frametime;

    for (auto i = vsync_begin + 1; i < vsync_end - 1; i++)
    {
        if (*i > *(i - 1))
            vsysc_frametime.push_back(*i - *(i - 1));
    }

    first_3_avg_frametime = (*vsysc_frametime.begin() + *(vsysc_frametime.begin() + 1)) / 2;

    // 获得标准framtime
    constexpr long frametime_30fps = 1000 * 1000 * 1000 / 30;
    constexpr long frametime_60fps = 1000 * 1000 * 1000 / 60;
    constexpr long frametime_90fps = 1000 * 1000 * 1000 / 90;
    constexpr long frametime_120fps = 1000 * 1000 * 1000 / 120;
    constexpr long frametime_144fps = 1000 * 1000 * 1000 / 144;

    if (first_3_avg_frametime > frametime_30fps * 9 / 10)
        first_3_avg_frametime = frametime_30fps;
    else if (first_3_avg_frametime > frametime_60fps * 9 / 10)
        first_3_avg_frametime = frametime_60fps;
    else if (first_3_avg_frametime > frametime_90fps * 9 / 10)
        first_3_avg_frametime = frametime_90fps;
    else if (first_3_avg_frametime > frametime_120fps * 9 / 10)
        first_3_avg_frametime = frametime_120fps;
    else if (first_3_avg_frametime > frametime_144fps * 9 / 10)
        first_3_avg_frametime = frametime_144fps;

    for (const auto &i : vsysc_frametime)
    {
        if (i > 10000000000)
            continue;
        if (i >= first_3_avg_frametime * 2)
            Jdata.big_jank_count++;
        else if (i >= first_3_avg_frametime * 11 / 10)
            Jdata.jank_count++;
        if (i <= first_3_avg_frametime * 2 / 3)
            Jdata.jank_count--;
    }

    return Jdata;
}
