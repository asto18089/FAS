#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

struct FtimeStamps
{
    vector<unsigned long> start_time_stamps;
    vector<unsigned long> vsync_time_stamps;
    vector<unsigned long> end_time_stamps;
    
    FtimeStamps()
    {
        start_time_stamps.reserve(10);
        vsync_time_stamps.reserve(10);
        end_time_stamps.reserve(10);
    }
};

string getSurfaceview();
FtimeStamps getOriginalData();
