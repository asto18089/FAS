#include <iostream>
#include <array>
#include <numeric>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

enum FRAMETIMES {
    FRAMETIME_30FPS = 1000 * 1000 * 1000 / 30,
    FRAMETIME_45FPS = 1000 * 1000 * 1000 / 45,
    FRAMETIME_60FPS = 1000 * 1000 * 1000 / 60,
    FRAMETIME_90FPS = 1000 * 1000 * 1000 / 90,
    FRAMETIME_120FPS = 1000 * 1000 * 1000 / 120,
    FRAMETIME_144FPS = 1000 * 1000 * 1000 / 144
};
const std::array<unsigned long, 6> standard_frametimes {FRAMETIME_30FPS, FRAMETIME_45FPS, FRAMETIME_60FPS, FRAMETIME_90FPS, FRAMETIME_120FPS, FRAMETIME_144FPS};

static unsigned long find_nearest_standard_frametime(const unsigned long& current_frametime) {
    size_t left = 0, right = std::size(standard_frametimes) - 1, mid;
    
    while (left <= right)
    {
        mid = (left + right) >> 1;

        if (current_frametime < standard_frametimes[mid])
            left = mid + 1;
        else
            right = mid - 1;
    }
    
    const auto& left_value = standard_frametimes[left];
    const auto& right_value = standard_frametimes[right];
    return (current_frametime - left_value < right_value - current_frametime) ? left_value : right_value;
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
    {
        Jdata.empty_private = true;
        return Jdata;
    }

    auto vsync_begin = Fdata.vsync_time_stamps.cbegin();
    auto vsync_end = Fdata.vsync_time_stamps.cend();

    vector<unsigned long> vsync_frametime;
    static unsigned long standard_frametime;

    for (auto i = vsync_begin + 1; i < vsync_end - 1; i++)
    {
        if (*i > *(i - 1))
            vsync_frametime.push_back(*i - *(i - 1));
    }

    standard_frametime = std::accumulate(vsync_frametime.cbegin(), vsync_frametime.cend(), 0) / vsync_frametime.size();
 
    // 获得标准frametime
    standard_frametime = find_nearest_standard_frametime(standard_frametime);

    for (auto &i : vsync_frametime)
    {
        switch (standard_frametime)
            /*case FRAMETIME_30FPS:
            {
                if (i / 100000 == 82 || i / 1000000 == 24)
                    i = standard_frametime;
                break;
            }
            case FRAMETIME_45FPS:
            {
                if (i / 100000 == 82 || i / 1000000 == 24)
                    i = standard_frametime;
                break;
            }*/
            case FRAMETIME_60FPS:
            {
                if (i / 100000 == 82 || i / 1000000 == 24)
                    i = standard_frametime;
                break;
            }
            /*case FRAMETIME_90FPS:
            {
                if (i / 100000 == 82 || i / 1000000 == 24)
                    i = standard_frametime;
                break;
            }
            case FRAMETIME_120FPS:
            {
                if (i / 100000 == 82 || i / 1000000 == 24)
                    i = standard_frametime;
                break;
            }
            case FRAMETIME_144FPS:
            {
                if (i / 100000 == 82 || i / 1000000 == 24)
                    i = standard_frametime;
                break;
            }*/
        
        std::cout << i << '\n';
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

bool jank_data::empty() const
{
    return this->empty_private;
}