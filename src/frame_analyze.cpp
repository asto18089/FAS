#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <sys/prctl.h>
#include <chrono>

#include "include/frame_analyze.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::thread;
using std::ref;
using namespace std::chrono;
using namespace std::this_thread;

string getSurfaceview() {
    FILE *game = popen("dumpsys SurfaceFlinger --list", "r");
    
    char buffer[1024] = {0};
    
    if (game == nullptr) {
        perror("Failed");
        pclose(game);
        
        return string(); //It's empty
    }
    
    while (fgets (buffer, sizeof(buffer) , game) ) {
        string result = buffer;
        if (result.find("SurfaceView[") != string::npos && result.find("BLAST") != string::npos) {
            result.pop_back();
            return result;
        } // 安卓11以及以上用的方法
        
        if (result.find("SurfaceView -") != string::npos) {
            result.pop_back();
            return result;
        } // 安卓11以下的方法

        /*安卓9以下的方法还不一样，不过没有必要适配*/
    }
    
    return string();
}

FtimeStamps getOriginalData() {
    FtimeStamps Fdata;
    
    string cmd = "dumpsys SurfaceFlinger --latency \'" + getSurfaceview() + "\'";
    FILE *dumpsys = popen(cmd.c_str(), "r");
    
    char buffer[1024] = {0};
    
    while (fgets(buffer, sizeof(buffer), dumpsys)) {
        static string analyze;
        static unsigned long long timestamps[3] = {0};
        analyze = buffer;
        analyze.pop_back();
        
        static bool finded(false);
        for (size_t pos = 0, len = 0, i = 0, pos_b = 0; pos < analyze.length(); pos++) {
            
            const bool& isnumber = (analyze[pos] <= '9' && analyze[pos] >= '0');
            
            if (! finded && isnumber) {
                pos_b = pos;
                finded = true;
            } else if (finded && (! isnumber || pos == analyze.length() - 1)) {
                len = pos - pos_b + 1;
                finded = false;
                
                timestamps[i] = atoll(analyze.substr(pos_b, len).c_str());
                i++;
            }
        }
        
        for (auto i : timestamps) {
            if (i == 0) {
                Fdata.start_time_stamps.clear();
                Fdata.vsync_time_stamps.clear();
                Fdata.end_time_stamps.clear();
                goto ANALYZE_END;
            } else if (i >= 99999999999999) {
                goto ANALYZE_END;
            }
        }
                
        for (size_t i = 0; i < 3; i++) {
            switch (i) {
            case 0:
                Fdata.start_time_stamps.push_back(timestamps[i]);
                break;
            case 1:
                Fdata.vsync_time_stamps.push_back(timestamps[i]);
                break;
            case 2:
                Fdata.end_time_stamps.push_back(timestamps[i]);
            }
        }

        ANALYZE_END:
        continue;
    }
    
    system("dumpsys SurfaceFlinger --latency-clear > /dev/null 2>&1");
    
    return Fdata;
}