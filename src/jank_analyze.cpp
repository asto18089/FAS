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

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"
#include "include/log.h"
#include "include/misc.h"

using namespace std::chrono;
using namespace std::this_thread;

#define INLINE_FT(x) constexpr unsigned int FRAMETIME_##x##FPS = 1000 * 1000 * 1000 / (x)

INLINE_FT(30);
INLINE_FT(45);
INLINE_FT(60);
INLINE_FT(90);
INLINE_FT(120);
INLINE_FT(144);

constexpr std::array<unsigned int, 6> STANDARD_FRAMETIMES{FRAMETIME_30FPS, FRAMETIME_45FPS, FRAMETIME_60FPS, FRAMETIME_90FPS, FRAMETIME_120FPS, FRAMETIME_144FPS};

static unsigned int find_nearest_standard_frametime(unsigned int current_frametime)
{
    static auto stamp = steady_clock::now();
    static int result = 0;
    if (duration_cast<milliseconds>(steady_clock::now() - stamp) < 10s && result != 0)
        return result;
    DEBUG("Start finding standard frametime");

    auto it = std::upper_bound(STANDARD_FRAMETIMES.rbegin(), STANDARD_FRAMETIMES.rend(), current_frametime);

    if (it == STANDARD_FRAMETIMES.rend())
    {
        DEBUG("Finded standard frametime :" + std::to_string(STANDARD_FRAMETIMES.front()));
        result = STANDARD_FRAMETIMES.front();
    }
    else if (it == STANDARD_FRAMETIMES.rbegin())
    {
        DEBUG("Finded standard frametime :" + std::to_string(STANDARD_FRAMETIMES.back()));
        result = STANDARD_FRAMETIMES.back();
    }
    else
    {
        auto left_value = *it;
        auto right_value = *(it - 1);
        result = (std::abs(static_cast<int>(current_frametime) - static_cast<int>(left_value)) < std::abs(static_cast<int>(right_value) - static_cast<int>(current_frametime))) ? left_value : right_value;
        
        DEBUG("Finded standard frametimes :" + std::to_string(left_value) + " and " + std::to_string(right_value));
    }
    stamp = steady_clock::now();
    return result;
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
    DEBUG("Start dumping frametimedata");
    jank_data Jdata;

    if (Fdata.vsync_timestamps.size() < 4)
    {
        Jdata.empty_private = true;
        DEBUG("Useless frametime data");
        sleep_for(300ms);
        return Jdata;
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

    standard_frametime = mode(vsync_frametime) * 1000 * 1000;

    // 获得标准frametime
    standard_frametime = find_nearest_standard_frametime(standard_frametime);
    
    DEBUG("standard_frametime :" + std::to_string(standard_frametime));
    
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
            Jdata.OOT++;
        else
            Jdata.LOT++;
    }
    if (standard_frametime > flashtime)
        sleep_for(milliseconds(standard_frametime / 1000 / 1000) * 2 * 10);
    else
        sleep_for(milliseconds(standard_frametime / 1000 / 1000) * 10);
    DEBUG("Dumped frametimedata");
    return Jdata;
}

double jank_data::nice() const
{
    return (double)OOT / (double)(OOT + LOT);
}

bool jank_data::empty() const
{
    return empty_private;
}
