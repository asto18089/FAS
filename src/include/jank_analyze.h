#pragma once

#include <vector>
using std::vector;

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
