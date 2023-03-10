#include <iostream>
#include <array>
#include <string>
#include <unistd.h>
#include <chrono>
#include <numeric>
#include <string_view>
#include <charconv>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

using namespace std::chrono;

constexpr unsigned int FRAMETIME_30FPS = 1000 * 1000 * 1000 / 30;
constexpr unsigned int FRAMETIME_45FPS = 1000 * 1000 * 1000 / 45;
constexpr unsigned int FRAMETIME_60FPS = 1000 * 1000 * 1000 / 60;
constexpr unsigned int FRAMETIME_90FPS = 1000 * 1000 * 1000 / 90;
constexpr unsigned int FRAMETIME_120FPS = 1000 * 1000 * 1000 / 120;
constexpr unsigned int FRAMETIME_144FPS = 1000 * 1000 * 1000 / 144;

constexpr std::array<unsigned int, 6> STANDARD_FRAMETIMES{FRAMETIME_30FPS, FRAMETIME_45FPS, FRAMETIME_60FPS, FRAMETIME_90FPS, FRAMETIME_120FPS, FRAMETIME_144FPS};

static unsigned long find_nearest_standard_frametime(unsigned long current_frametime)
{
    size_t left = 0, right = STANDARD_FRAMETIMES.size() - 1, mid = 0;

    while (left <= right)
    {
        mid = (left + right) >> 1;
        if (left >= STANDARD_FRAMETIMES.size() - 1)
        {
            left = STANDARD_FRAMETIMES.size() - 1;
            break;
        }

        if (right <= 0)
        {
            right = 0;
            left = 0;
            break;
        }

        if (current_frametime < STANDARD_FRAMETIMES[mid])
            left = mid + 1;
        else
            right = mid - 1;
    }

    auto left_value = STANDARD_FRAMETIMES[left];
    auto right_value = STANDARD_FRAMETIMES[right];
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

    if (Fdata.vsync_timestamps.size() < 4)
    {
        Jdata.empty_private = true;
        return Jdata;
    }

    auto vsync_begin = Fdata.vsync_timestamps.cbegin();
    auto vsync_end = Fdata.vsync_timestamps.cend();

    vector<unsigned long> vsync_frametime;
    static unsigned long standard_frametime;

    for (auto i = vsync_begin + 1; i < vsync_end - 1; i++)
    {
        if (*i > *(i - 1))
            vsync_frametime.push_back(*i - *(i - 1));
    }

    standard_frametime = mode(vsync_frametime) * 1000 * 1000;

    // 获得标准frametime
    standard_frametime = find_nearest_standard_frametime(standard_frametime);

    // std::cout << standard_frametime << '\n';
    auto getRefreshRate = []()
    {
        static int result = 0;
        static auto stamp = steady_clock::now();
        
        if (duration_cast<milliseconds>(steady_clock::now() - stamp) < 5s && result != 0)
            return result;
        
        FILE *dumpsys = popen("dumpsys SurfaceFlinger 2>/dev/null", "r");
        if (dumpsys == nullptr)
        {
            pclose(dumpsys);
            perror("Failed");

            return result;
        }
        char buffer[1024] = {'\0'};

        while (fgets(buffer, sizeof(buffer), dumpsys))
        {
            const std::string_view analyze = buffer;
            if (analyze.find("refresh-rate") != std::string_view::npos)
            {
                const size_t start = analyze.find(':') + 2;
                const size_t end = analyze.find('.', start + 1);
                std::from_chars(analyze.data() + start, analyze.data() + end, result);
                break;
            }
        }

        pclose(dumpsys);
        stamp = steady_clock::now();
        
        return result;
    };
    
    const unsigned long flashtime = getRefreshRate() != 0 ? 1000 * 1000 * 1000 / getRefreshRate() : 0;
    for (auto &i : vsync_frametime)
    {
        if (standard_frametime > flashtime)
        {
            const unsigned long high_ignore = standard_frametime * 2 - flashtime;

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
