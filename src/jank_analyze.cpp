#include <iostream>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

#define FRAMETIME_30FPS (1000 * 1000 * 1000 / 30)
#define FRAMETIME_45FPS (1000 * 1000 * 1000 / 45)
#define FRAMETIME_60FPS (1000 * 1000 * 1000 / 60)
#define FRAMETIME_90FPS (1000 * 1000 * 1000 / 90)
#define FRAMETIME_120FPS (1000 * 1000 * 1000 / 120)
#define FRAMETIME_144FPS (1000 * 1000 * 1000 / 144)
const long standard_frametimes[] = {FRAMETIME_30FPS, FRAMETIME_45FPS, FRAMETIME_60FPS, FRAMETIME_90FPS, FRAMETIME_120FPS, FRAMETIME_144FPS};

static long find_nearest_standard_frametime(const long& current_frametime) {
    int left = 0;
    int right = sizeof(standard_frametimes) / sizeof(long) - 1;
    int mid;
    
    while (left <= right)
    {
        mid = (left + right) >> 1;
        
        if (current_frametime >= standard_frametimes[mid] * 9 / 10)
            left = mid + 1;
        else
            right = mid - 1;
    }

    return standard_frametimes[left];
}

/* 让游戏始终运行在刚好(差点)满足需要的频率上
   需要让始终保持发生一定数量轻微的超时
   如果frametime小于该(需要)超时后的frametime
   则说明性能余量过多
   如果frametime大于该frametime
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
            vsync_frametime.push_back(*i - *(i - 1));
    }

    standard_frametime = (*vsync_frametime.cbegin() + *(vsync_frametime.cbegin() + 1)) / 2;

    // 获得标准frametime
    standard_frametime = find_nearest_standard_frametime(standard_frametime);

    for (const auto &i : vsync_frametime)
    {
        if (i > standard_frametime)
            Jdata.OOT++;
        else
            Jdata.LOT++;
    }

    return Jdata;
}

double jank_data::nice() const
{
    return (double)(this->OOT) / (double)(this->OOT + this->LOT);
}