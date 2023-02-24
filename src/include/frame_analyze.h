#pragma once

#include <vector>

using std::string;
using std::vector;

struct FtimeStamps
{
    vector<unsigned long> start_time_stamps;
    vector<unsigned long> vsync_time_stamps;
    vector<unsigned long> end_time_stamps;
};

string getSurfaceview();
FtimeStamps getOriginalData();
