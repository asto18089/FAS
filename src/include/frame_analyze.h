#pragma once

#include <string>
#include <vector>
#include <thread>

using std::string;
using std::vector;
using namespace std::this_thread;

struct FtimeStamps
{
    vector<unsigned long> start_timestamps;
    vector<unsigned long> vsync_timestamps;
    vector<unsigned long> end_timestamps;
    static int fps;

    FtimeStamps()
    {
        start_timestamps.reserve(10);
        vsync_timestamps.reserve(10);
        end_timestamps.reserve(10);
        getFps();
    }
    static void fpsWatcher();
    static void getFps()
    {
        static std::thread thread_fps(fpsWatcher);
        thread_fps.detach();
    }
    void getOriginalData();
};

string getSurfaceview();
