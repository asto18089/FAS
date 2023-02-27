#include <vector>
#include <iostream>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

using std::cout;
using std::endl;
using std::vector;

jank_data analyzeFrameData(const FtimeStamps &Fdata)
{
    jank_data Jdata;

    if (Fdata.start_time_stamps.size() < 4)
    {
        Jdata.big_jank_count = 66;
        Jdata.jank_count = 66;
        return Jdata;
    }

    auto start_begin = Fdata.start_time_stamps.cbegin();
    auto start_end = Fdata.start_time_stamps.cend();

    vector<unsigned long> start_frametime;
    static unsigned long first_3_avg_frametime;

    for (auto i = start_begin + 1; i < start_end - 1; i++)
    {
        if (*i > *(i - 1))
            start_frametime.push_back(*i - *(i - 1));
    }

    first_3_avg_frametime = (*start_frametime.cbegin() + *(start_frametime.cbegin() + 1)) / 2;

    // 获得标准frametime
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

    for (const auto &i : start_frametime)
    {
        cout << i << '\n';
        if (i >= first_3_avg_frametime * 2)
            Jdata.big_jank_count++;
        else if (i >= first_3_avg_frametime * 11 / 10)
            Jdata.jank_count++;
    }
    cout << endl;

    return Jdata;
}
