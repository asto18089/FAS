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

    FtimeStamps()
    {
        start_timestamps.reserve(10);
        vsync_timestamps.reserve(10);
        end_timestamps.reserve(10);
        getOriginalData();
        getFps();
    }
    static void fpsWatcher(int&);
    static int getFps()
    {
        static int fps = 0;
        static std::thread thread_fps(fpsWatcher, std::ref(fps));
        static bool detached = false;
        if (!detached) //只在第一次调用时分离线程
        {
            thread_fps.detach();
            detached = true;
        }
        return fps;
    }
    void getOriginalData();
};

string getSurfaceview();