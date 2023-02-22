#include <vector>
#include <iostream>

#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;

jank_data analyzeFrameData(const FtimeStamps& Fdata) {
    jank_data Jdata;
    
    if (Fdata.vsync_time_stamps.size() < 4) {
        return Jdata;
    }
    
    const unsigned long long MOVIE_FRAME_TIME = 1000 * 1000 * 1000 / 24;

    const auto &vsync_begin = Fdata.vsync_time_stamps.begin();
    const auto &vsync_end = Fdata.vsync_time_stamps.end();
    
    vector <unsigned long long>vsysc_frametime;
    static unsigned long long first_3_avg_frametime;
    
    for (auto i = vsync_begin + 1; i < vsync_end - 1; i++) {
        vsysc_frametime.push_back(*i - *(i - 1));
    }
    
    first_3_avg_frametime = *(vsysc_frametime.begin()) + *(vsysc_frametime.begin() + 1) / 2;
    cout << first_3_avg_frametime << endl;
    
    for (auto i : vsysc_frametime) {
        const bool& bg_movie_2 = (i >= 2 * MOVIE_FRAME_TIME);
        const bool& bg_movie_3 = (i >= 3 * MOVIE_FRAME_TIME);
        
        if (bg_movie_2 && i > first_3_avg_frametime) {
            Jdata.jank_count++;
        } else if (bg_movie_3 && i > first_3_avg_frametime) {
            Jdata.big_jank_count++;
        }
    }
    
    return Jdata;
}
