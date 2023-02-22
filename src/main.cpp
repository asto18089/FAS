#include <sched.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <chrono>
#include <string>
#include <thread>

#include "include/cpufreq.h"
#include "include/frame_analyze.h"
#include "include/jank_analyze.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;
using std::string;
using namespace std::chrono;
using namespace std::this_thread;

const char *PROFILE_LOCATION = "/sdcard/Android/FAS/FAS.conf";

void bound2little() {
    cpu_set_t mask;
    CPU_ZERO(&mask);

    CPU_SET(0, &mask);
    CPU_SET(1, &mask);
    CPU_SET(2, &mask);
    CPU_SET(3, &mask);
    
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
}

int main() {
    bound2little();
    
    Cpufreq cpu_controller;
    
    LOOP: // 主循环线程，死循环
    while (getSurfaceview().empty()) {
        sleep_for(seconds(1));
    }
    
    jank_data jdata = analyzeFrameData(getOriginalData());
    
    cout << "Jank count" << jdata.jank_count << endl;
    cout << "Big Jank count" << jdata.big_jank_count << endl;
    
    sleep_for(milliseconds(400));
    
    goto LOOP; // LOOP never ends
}