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
    
    //const unsigned long long MOVIE_FRAME_TIME = 1000 * 1000 * 1000 / 24;

    const auto &vsync_begin = Fdata.vsync_time_stamps.begin();
    const auto &vsync_end = Fdata.vsync_time_stamps.end();
    
    /*for (auto i : Fdata.vsync_time_stamps) {
        cout << i << endl;
    }*/
    
    vector <unsigned long long>vsysc_frametime;
    static unsigned long long first_3_avg_frametime;
    
    for (auto i = vsync_begin + 1; i < vsync_end - 1; i++) {
        if (*i > *(i - 1)) {
            vsysc_frametime.push_back(*i - *(i - 1));
        }
    }
    
    first_3_avg_frametime = (*vsysc_frametime.begin() + *(vsysc_frametime.begin() + 1)) / 2;
    
    //cout << first_3_avg_frametime << endl;
    
    for (auto i : vsysc_frametime) {
        if (i > 1000000000) {
            continue;
        }
        
        if (i > 1.3 * first_3_avg_frametime) {
            Jdata.big_jank_count++;
            for (auto it : vsysc_frametime) {
                if ((i + it) / 2 < 1.3 * first_3_avg_frametime) {
                    Jdata.big_jank_count--;
                    break;
                }
            }
        } else if (i > 1.1 * first_3_avg_frametime) {
            Jdata.jank_count++;
            for (auto it : vsysc_frametime) {
                if ((i + it) / 2 < 1.1 * first_3_avg_frametime) {
                    Jdata.jank_count--;
                    break;
                }
            }
        }
    }
    
    return Jdata;
}
