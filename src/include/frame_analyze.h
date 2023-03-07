#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

struct FtimeStamps
{
    vector<unsigned long> start_timestamps;
    vector<unsigned long> vsync_timestamps;
    vector<unsigned long> end_timestamps;

    FtimeStamps()
    {
        start_timestamps.reserve(10);
        vsync_timestamps.reserve(10);
        end_timestamps.reserve(10);
    }
};

string getSurfaceview();
FtimeStamps getOriginalData();
