#pragma once

struct jank_data
{
/* NOT : needed out of time
   OOT : others of frametime*/
    int NOT = 0;
    int OOT = 0;

    float nice() const;
};

jank_data analyzeFrameData(const FtimeStamps &Fdata);
