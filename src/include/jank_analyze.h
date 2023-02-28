#pragma once

struct jank_data
{
    int OOT = 0;
    int LOT = 0;

    double nice() const;
};

jank_data analyzeFrameData(const FtimeStamps &Fdata);
