
#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

void analyzeFrameData(const FtimeStamps& Fdata) {
    static unsigned long long first_3_avg_frametime;
    const unsigned long long MOVIE_FRAME_TIME = 1000 * 1000 * 1000 / 24
    
    auto &vsync_begin = Fdata.vsync_time_stamps.begin();
    for (auto i = vsync_begin + 1; i < vsync_begin + 2; i++) {
        first_3_avg_frametime += *i - *(i - 1);
    }
    first_3_avg_frametime = first_3_avg_frametime / 2;
    
    
    
}
