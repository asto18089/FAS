#pragma once

struct jank_data
{
/* NOT : needed out of time
   OOT : out of NOT
   LOT : less than NOT*/
    int NOT = 0;
    int OOT = 0;
    int LOT = 0;

    float nice() const;
    bool odd() const;
};

jank_data analyzeFrameData(const FtimeStamps &Fdata);
