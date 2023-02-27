#include <vector>
#include <iostream>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

using std::cout;
using std::endl;
using std::vector;

/* 让游戏始终运行在刚好(差点)满足需要的频率上
   需要让frametime始终保持轻微的超时
   如果framtime小于该(需要)超时后的frametime
   则说明性能余量过多
   如果framtime大于该frametime
   则说明需要更多性能
   如此可得到游戏运行刚好需要的频率 */

jank_data analyzeFrameData(const FtimeStamps &Fdata)
{
    jank_data Jdata;

    if (Fdata.start_time_stamps.size() < 4)
        return Jdata;

    auto start_begin = Fdata.start_time_stamps.cbegin();
    auto start_end = Fdata.start_time_stamps.cend();

    vector<unsigned long> start_frametime;
    static unsigned long standard_frametime, needed_frametime;

    for (auto i = start_begin + 1; i < start_end - 1; i++)
    {
        if (*i > *(i - 1))
            start_frametime.emplace_back(*i - *(i - 1));
    }

    standard_frametime = (*start_frametime.cbegin() + *(start_frametime.cbegin() + 1)) / 2;

    // 获得标准frametime
    constexpr long frametime_30fps = 1000 * 1000 * 1000 / 30;
    constexpr long n_frametime_30fps = 1000 * 1000 * 1000 / 27;
    constexpr long frametime_60fps = 1000 * 1000 * 1000 / 60;
    constexpr long n_frametime_60fps = 1000 * 1000 * 1000 / 57;
    constexpr long frametime_90fps = 1000 * 1000 * 1000 / 90;
    constexpr long n_frametime_90fps = 1000 * 1000 * 1000 / 87;
    constexpr long frametime_120fps = 1000 * 1000 * 1000 / 120;
    constexpr long n_frametime_120fps = 1000 * 1000 * 1000 / 116;
    constexpr long frametime_144fps = 1000 * 1000 * 1000 / 144;
    constexpr long n_frametime_144fps = 1000 * 1000 * 1000 / 140;

    if (standard_frametime > frametime_30fps * 9 / 10)
        needed_frametime = n_frametime_30fps;
    else if (standard_frametime > frametime_60fps * 9 / 10)
        needed_frametime = n_frametime_60fps;
    else if (standard_frametime > frametime_90fps * 9 / 10)
        needed_frametime = n_frametime_90fps;
    else if (standard_frametime > frametime_120fps * 9 / 10)
        needed_frametime = n_frametime_120fps;
    else if (standard_frametime > frametime_144fps * 9 / 10)
        needed_frametime = n_frametime_144fps;
        

    for (const auto &i : start_frametime)
    {
        cout << i << '\n';
        
        if (i > needed_frametime)
            Jdata.OOT++;
        else if (i < standard_frametime * 14 / 15)
            Jdata.LOT++;
        else
            Jdata.NOT++;
    }
    cout << endl;

    return Jdata;
}

float jank_data::nice() const
{
    return this->NOT / (this->NOT + this->OOT + this->LOT);
}

bool jank_data::odd() const
{
    return (OOT > LOT);
}