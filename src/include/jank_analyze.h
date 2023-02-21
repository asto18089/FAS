#pragma once

struct jank_data {
    unsigned short jank_count;
    unsigned short big_jank_count;
    jank_data() : jank_count(0), big_jank_count(0) {};
}