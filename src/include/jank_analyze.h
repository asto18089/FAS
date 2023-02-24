#pragma once

struct jank_data
{
    short jank_count = 0;
    short big_jank_count = 0;
};

jank_data analyzeFrameData(const FtimeStamps &Fdata);
