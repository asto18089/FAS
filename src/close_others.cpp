#include <unistd.h>
#include <thread>
#include <chrono>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#include "include/close_others.h"

using std::thread;
using std::ofstream;
using std::ios;
using namespace std::chrono;
using namespace std::this_thread;

// Edit and Lock a file
template <typename T>
bool Lockvalue(const char* location, T value)
{
    chmod(location, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH);
    ofstream fd(location, ios::out | ios::trunc);
    if (!fd)
    {
        fd.close();
        return false;
    }
    fd << value;
    fd.close();
    chmod(location, S_IRUSR | S_IRGRP | S_IROTH);
    return true;
}

static void close_others() {
    prctl(PR_SET_NAME, "close_others");
    
    DAEMON:
    system("stop fpsgo");
    Lockvalue("/sys/module/mtk_fpsgo/parameters/perfmgr_enable", 0);
    Lockvalue("/sys/kernel/fpsgo/common/fpsgo_enable", 0);
    Lockvalue("/sys/module/perfmgr/parameters/perfmgr_enable", 0);
    Lockvalue("/sys/module/perfmgr_policy/parameters/perfmgr_enable", 0);
    sleep(5);
    goto DAEMON;
}

void start_close_others() {
    thread t_close_others(close_others);
    t_close_others.detach();
}