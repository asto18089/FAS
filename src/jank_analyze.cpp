#include <iostream>
#include <array>
#include <string>
#include <unistd.h>
#include <numeric>
#include <map>

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
const std::array<unsigned long, 6> STANDARD_FRAMETIMES { FRAMETIME_30FPS, FRAMETIME_45FPS, FRAMETIME_60FPS, FRAMETIME_90FPS, FRAMETIME_120FPS, FRAMETIME_144FPS };

static unsigned long find_nearest_standard_frametime(const unsigned long& current_frametime) {
    ssize_t left = 0, right = std::size(STANDARD_FRAMETIMES) - 1, mid;
    
    while (left <= right)
    {
        mid = (left + right) >> 1;
        if (left >= std::size(STANDARD_FRAMETIMES) - 1)
        {
            left = std::size(STANDARD_FRAMETIMES) - 1;
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
    
    const auto& left_value = STANDARD_FRAMETIMES[left];
    const auto& right_value = STANDARD_FRAMETIMES[right];
    return (current_frametime - left_value < right_value - current_frametime) ? left_value : right_value;
}

template<typename T>
static T mode(const vector<T>& v) {
    // 创建一个map，键为元素值，值为出现次数
    std::map<T, int> m;
  
    // 遍历vector，更新map中的计数
    for (const auto& x : v) {
        m[x / (1000 * 1000)]++;
    }
  
    // 初始化众数和最大次数
    T mode = v[0];
    int max_count = m[v[0]];
  
    // 遍历map，找到最大次数对应的元素
    for (const auto& p : m)
    {
        if (p.second > max_count)
        {
            mode = p.first;
            max_count = p.second;
        }
    }
  
    // 返回众数
    return mode;
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

    standard_frametime = mode(vsync_frametime) * 1000 * 1000;
 
    // 获得标准frametime
    standard_frametime = find_nearest_standard_frametime(standard_frametime);
    
    // std::cout << standard_frametime << '\n';
    auto getFlashrate = []()
    {
        FILE *dumpsys = popen("dumpsys SurfaceFlinger", "r");
        char buffer[1024] = {0};
        int result = 0;
        
        while (fgets(buffer, sizeof(buffer), dumpsys))
        {
            std::string analyze = buffer;
            analyze.pop_back();
            
            if (analyze.find("refresh-rate") != std::string::npos)
            {
                const size_t& pos = analyze.find(':');
                const size_t& len = analyze.find('.', pos + 1) - pos;
                result = stoi(analyze.substr(pos, len));
                break;
            }
        }
        
        pclose(dumpsys);
        return result;
    };
    auto ingorneMismatch = [&](unsigned long& frametime)
    {
        const unsigned long& flashtime = 1000 * 1000 * 1000 / getFlashrate();
        if (standard_frametime <= flashtime)
            return;
            
        const auto& high_ingorne = standard_frametime - flashtime;
        
        const std::string& s_highingorne = std::to_string(high_ingorne);
        const std::string& s_lowingorne = std::to_string(flashtime);
        const std::string& s_frametime = std::to_string(frametime);
        
        if (s_frametime.length() != s_highingorne.length() && s_frametime.length() != s_lowingorne.length())
            return;
            
        auto same = [](const std::string& a, const std::string& b)
        {
            return (*a.cbegin() == *b.cbegin() && *(++a.cbegin()) == *(++b.cbegin()));
        };
        
        if (same(s_frametime, s_highingorne) || same(s_frametime, s_lowingorne))
            frametime = standard_frametime;
    };
    
    for (auto &i : vsync_frametime)
    {
        ingorneMismatch(i);
        
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