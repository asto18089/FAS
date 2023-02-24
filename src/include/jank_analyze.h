#pragma once

struct jank_data {
    short jank_count;
    short big_jank_count;
    jank_data() : jank_count(0), big_jank_count(0) {}
};

jank_data analyzeFrameData(const FtimeStamps& Fdata);