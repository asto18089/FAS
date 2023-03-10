#pragma once

#include <unordered_map>

template <typename T>
constexpr static T mode(const vector<T> &v)
{
    // 创建一个unordered_map，键为元素值，值为出现次数
    std::unordered_map<T, int> m;

    // 遍历vector，更新unordered_map中的计数
    for (const auto &x : v)
        m[x / (1000 * 1000)]++;

    // 初始化众数和最大次数
    T mode = v[0];
    int max_count = m[v[0]];

    // 遍历unordered_map，找到最大次数对应的元素
    for (const auto &p : m)
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

struct jank_data
{
private:
    bool empty_private = false;

public:
    int OOT = 0;
    int LOT = 0;

    double nice() const;
    bool empty() const;

    friend jank_data analyzeFrameData(const FtimeStamps &Fdata);
};

jank_data analyzeFrameData(const FtimeStamps &Fdata);
