#include <vector>
#include <iostream>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

using std::cout;
using std::endl;
using std::vector;

/* 让游戏始终运行在刚好(差点)满足需要的频率上
   需要让始终保持发生一定数量轻微的超时
   如果framtime小于该(需要)超时后的frametime
   则说明性能余量过多
   如果framtime大于该frametime
   则说明需要更多性能
   如此可得到游戏运行刚好需要的频率 */

jank_data analyzeFrameData(const FtimeStamps &Fdata)
{
    jank_data Jdata;

    if (Fdata.vsync_time_stamps.size() < 4)
        return Jdata;

    auto vsync_begin = Fdata.vsync_time_stamps.cbegin();
    auto vsync_end = Fdata.vsync_time_stamps.cend();

    vector<unsigned long> vsync_frametime;
    static unsigned long standard_frametime;

    for (auto i = vsync_begin + 1; i < vsync_end - 1; i++)
    {
        if (*i > *(i - 1))
            vsync_frametime.emplace_back(*i - *(i - 1));
    }

    standard_frametime = (*vsync_frametime.cbegin() + *(vsync_frametime.cbegin() + 1)) / 2;

    // 获得标准frametime
    constexpr long frametime_30fps = 1000 * 1000 * 1000 / 30;
    constexpr long frametime_45fps = 1000 * 1000 * 1000 / 45;
    constexpr long frametime_60fps = 1000 * 1000 * 1000 / 60;
    constexpr long frametime_90fps = 1000 * 1000 * 1000 / 90;   
    constexpr long frametime_120fps = 1000 * 1000 * 1000 / 120;  
    constexpr long frametime_144fps = 1000 * 1000 * 1000 / 144;

    if (standard_frametime > frametime_30fps * 9 / 10)
        standard_frametime = frametime_30fps;
    if (standard_frametime > frametime_45fps * 9 / 10)
        standard_frametime = frametime_45fps;
    else if (standard_frametime > frametime_60fps * 9 / 10)
        standard_frametime = frametime_60fps;
    else if (standard_frametime > frametime_90fps * 9 / 10)
        standard_frametime = frametime_90fps;
    else if (standard_frametime > frametime_120fps * 9 / 10)
        standard_frametime = frametime_120fps;
    else if (standard_frametime > frametime_144fps * 9 / 10)
        standard_frametime = frametime_144fps;
        

    for (const auto &i : vsync_frametime)
    {
        // cout << i << '\n';
        
        if (i > standard_frametime)
            Jdata.OOT++;
        else
            Jdata.LOT++;
    }

    return Jdata;
}

float jank_data::nice() const
{
    return (float)(this->OOT) / (float)(this->OOT + this->LOT);
}