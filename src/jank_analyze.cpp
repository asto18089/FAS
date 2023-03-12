#include <iostream>
#include <array>
#include <string>
#include <sstream>
#include <unistd.h>
#include <chrono>
#include <numeric>
#include <string_view>
#include <charconv>
#include <thread>
#include <unordered_map>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"
#include "include/log.h"
#include "include/misc.h"

using namespace std::chrono;
using namespace std::this_thread;

constexpr std::array<std::pair<int, int>, 6> FPS_FRAMETIMES
{
    {
        {30, 1000 * 1000 * 1000 / 30},
        {45, 1000 * 1000 * 1000 / 45},
        {60, 1000 * 1000 * 1000 / 60},
        {90, 1000 * 1000 * 1000 / 90},
        {120, 1000 * 1000 * 1000 / 120},
        {144, 1000 * 1000 * 1000 / 144}
    }
};

static std::pair<int, int> find_nearest_standard(int current_fps) // 通过当前fps找出标准帧渲染时间
{
    if (current_fps <= FPS_FRAMETIMES.cbegin()->first + 3)
        return *FPS_FRAMETIMES.cbegin();

    if (current_fps >= (FPS_FRAMETIMES.cend() - 1)->first)
        return *(FPS_FRAMETIMES.cend() - 1);

    for (auto i = FPS_FRAMETIMES.cbegin(); i < FPS_FRAMETIMES.cend() - 1; i++)
    {
        if (i->first < current_fps && (i + 1)->first + 4 > current_fps) // 4 是fps可能的误差
        {
            return *(i + 1);
        }
    }
    return {};
}

Jank_data::Jank_data(const FtimeStamps &Fdata)
{
    analyzeFrameData(Fdata);
}

/* 让游戏始终运行在刚好(差点)满足需要的频率上
   需要让始终保持发生一定数量轻微的超时
   如果frametime小于该(需要)超时后的frametime
   则说明性能余量过多
   如果frametime大于该frametime
   则说明需要更多性能
   如此可得到游戏运行刚好需要的频率 */

void Jank_data::analyzeFrameData(const FtimeStamps &Fdata)
{
    DEBUG("Start dumping frametimedata");
    if (Fdata.vsync_timestamps.size() < 4)
    {
        empty_private = true;
        DEBUG("Useless frametime data");
        sleep_for(300ms);
        return;
    }

    auto vsync_begin = Fdata.vsync_timestamps.cbegin();
    auto vsync_end = Fdata.vsync_timestamps.cend();

    vector<unsigned int> vsync_frametime;
    static unsigned int standard_frametime;

    for (auto i = vsync_begin + 1; i < vsync_end - 1; i++)
    {
        if (*i > *(i - 1))
            vsync_frametime.push_back(*i - *(i - 1));
    }

    // 获得标准frametime
    const auto &standard = find_nearest_standard(Fdata.getFps());
    standard_frametime = standard.second;
    missed_fps = standard.first - Fdata.getFps() - 2;
    DEBUG("Missed fps :" + std::to_string(missed_fps));

    DEBUG("Standard frametime :" + std::to_string(standard_frametime));
    
    auto getRefreshRate = []()
    {
        static int result = 0;
        static auto stamp = steady_clock::now();
        
        if (duration_cast<milliseconds>(steady_clock::now() - stamp) < 5s && result != 0)
            return result;
        
        DEBUG("Start dumping freshrate");
        string dumpsys = execCmdSync("/system/bin/dumpsys", {"SurfaceFlinger"});
        if (dumpsys.empty())
            return result;
            
        std::istringstream iss(dumpsys);
        std::string analyze;
        while (std::getline(iss, analyze))
        {
            if (analyze.find("refresh-rate") != std::string_view::npos)
            {
                const size_t start = analyze.find(':') + 2;
                const size_t end = analyze.find('.', start + 1);
                std::from_chars(analyze.data() + start, analyze.data() + end, result);
                break;
            }
        }
        stamp = steady_clock::now();
        DEBUG("Dumped freshrate :" + std::to_string(result));
        return result;
    };
    
    const unsigned int flashtime = getRefreshRate() != 0 ? 1000 * 1000 * 1000 / getRefreshRate() : 0;
    for (auto &i : vsync_frametime)
    {
        if (standard_frametime > flashtime)
        {
            const unsigned int high_ignore = standard_frametime * 2 - flashtime;

            const std::string s_highignore = std::to_string(high_ignore);
            const std::string s_lowignore = std::to_string(flashtime);
            const std::string s_frametime = std::to_string(i);

            if (s_frametime.length() == s_highignore.length() || s_frametime.length() == s_lowignore.length())
            {
                auto same = [](const std::string &a, const std::string &b)
                {
                    if (a.length() <= 7)
                        return (*a.cbegin() == *b.cbegin());
                    return (*a.cbegin() == *b.cbegin() && *(++a.cbegin()) == *(++b.cbegin()));
                };

                if (same(s_frametime, s_highignore) || same(s_frametime, s_lowignore))
                    i = standard_frametime;
            }
        }

        // std::cout << i << '\n';
        if (i > standard_frametime)
            OOT++;
        else
            LOT++;
    }
    if (standard_frametime > flashtime)
        sleep_for(milliseconds(standard_frametime / 1000 / 1000) * 2 * 10);
    else
        sleep_for(milliseconds(standard_frametime / 1000 / 1000) * 10);
    DEBUG("Dumped frametimedata");
}

double Jank_data::nice() const
{
    return (double)OOT / (double)(OOT + LOT);
}

bool Jank_data::empty() const
{
    return empty_private;
}
