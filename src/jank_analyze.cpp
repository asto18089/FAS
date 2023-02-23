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
        Jdata.big_jank_count = 66;
        Jdata.jank_count = 66;
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
    
    for (auto i : vsysc_frametime) {
        first_3_avg_frametime += i;
    }
    first_3_avg_frametime = first_3_avg_frametime / vsysc_frametime.size();
    
    // 获得标准framtime
    const long long frametime_30fps = 1000 * 1000 * 1000 / 30;
    const long long frametime_60fps = 1000 * 1000 * 1000 / 60;
    const long long frametime_90fps = 1000 * 1000 * 1000 / 90;
    const long long frametime_120fps = 1000 * 1000 * 1000 / 120;
    const long long frametime_144fps = 1000 * 1000 * 1000 / 144;
    
    if (first_3_avg_frametime > frametime_30fps * 0.9) {
        first_3_avg_frametime = frametime_30fps;
    } else if (first_3_avg_frametime > frametime_60fps * 0.9) {
        first_3_avg_frametime = frametime_60fps;
    } else if (first_3_avg_frametime > frametime_90fps * 0.9) {
        first_3_avg_frametime = frametime_90fps;
    } else if (first_3_avg_frametime > frametime_120fps * 0.9) {
        first_3_avg_frametime = frametime_120fps;
    } else if (first_3_avg_frametime > frametime_144fps * 0.9) {
        first_3_avg_frametime = frametime_144fps;
    }
    
    for (auto &i : vsysc_frametime) {
        if (i > 1000000000) {
            continue;
        }
        
        if (i >= 1.3 * first_3_avg_frametime) {
            Jdata.big_jank_count++;
            for (auto &it : vsysc_frametime) {
                if ((i + it) / 2 < 1.3 * first_3_avg_frametime) {
                    Jdata.big_jank_count--;
                    i = first_3_avg_frametime;
                    it = first_3_avg_frametime;
                    break;
                }
            }
        } else if (i >= 1.1 * first_3_avg_frametime) {
            Jdata.jank_count++;
            for (auto &it : vsysc_frametime) {
                if ((i + it) / 2 < 1.1 * first_3_avg_frametime) {
                    Jdata.jank_count--;
                    i = first_3_avg_frametime;
                    it = first_3_avg_frametime;
                    break;
                }
            }
        }
    }
    
    return Jdata;
}
