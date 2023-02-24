#include <unistd.h>
#include <thread>
#include <chrono>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#include "include/close_others.h"

using std::ios;
using std::ofstream;
using std::thread;

static void close_others()
{
    prctl(PR_SET_NAME, "close_others");

    while (true)
    {
        system("stop fpsgo");
        Lockvalue("/sys/module/mtk_fpsgo/parameters/perfmgr_enable", 0);
        Lockvalue("/sys/kernel/fpsgo/common/fpsgo_enable", 0);
        Lockvalue("/sys/module/perfmgr/parameters/perfmgr_enable", 0);
        Lockvalue("/sys/module/perfmgr_policy/parameters/perfmgr_enable", 0);
        sleep(5);
    }
}

void start_close_others()
{
    thread t_close_others(close_others);
    t_close_others.detach();
}
