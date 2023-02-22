#pragma once

#include <vector>

using std::vector;
using std::string;

struct FtimeStamps {
    vector <unsigned long long>start_time_stamps;
    vector <unsigned long long>vsync_time_stamps;
    vector <unsigned long long>end_time_stamps;
};

string getSurfaceview();
FtimeStamps getOriginalData();