#include <vector>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

using std::vector;

jankdata analyzeFrameData(const FtimeStamps& Fdata) {
    jankdata Jdata;
    
    const unsigned long long MOVIE_FRAME_TIME = 1000 * 1000 * 1000 / 24;

    const auto &vsync_begin = Fdata.vsync_time_stamps.begin();
    const auto &vsync_end = Fdata.vsync_time_stamps.end();
    
    vector <unsigned long long>vsysc_frametime;
    static unsigned long long first_3_avg_frametime;
    
    for (auto i = vsync_begin + 1; i < vsync_end - 1; i++) {
        vsysc_frametime.push_back(*i - *(i - 1));
    }
    
    first_3_avg_frametime = *(vsysc_frametime.begin()) + *(vsysc_frametime.begin() + 1) / 2;
    
    for (auto i : vsysc_frametime) {
        bool& 2bg_movie = (i >= 2 * MOVIE_FRAME_TIME);
        bool& 3bg_movie = (i >= 3 * MOVIE_FRAME_TIME);
        
        if (2bg_movie && i > first_3_avg_frametime) {
            Jdata.jank_count++;
        } else if (3bg_movie && i > first_3_avg_frametime) {
            Jdata.big_jank_count++;
        }
    }
    
    return Jdata;
}
