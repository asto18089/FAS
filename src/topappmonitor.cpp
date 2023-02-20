#include <string>
#include <unistd.h>
#include <sys/prctl.h>
#include <thread>
#include <fstream>
#include <sys/types.h>

#include "include/topappmonitor.h"

using std::string;

static bool Shell (const char *sh, string &result) {
    FILE *pp = popen(sh, "r");
    if (pp == NULL)
    {
        perror("Failed");
        return false;
    }
    
    char buffer[1024];
    // collect result
    fgets(buffer, sizeof(buffer), pp);
    result = buffer;
    
    result.pop_back();
    pclose(pp);
    return true;
}

static bool Testfile (const char* location) {
    int ret = access(location, F_OK);
    return (ret == 0);
}


static string getTopapp() {
    if (Testfile("/sys/kernel/gbe/gbe2_fg_pid"))
    {
        string pid;
        std::ifstream Pid("/sys/kernel/gbe/gbe2_fg_pid"), app;
        Pid >> pid;
        Pid.close();
        
        app.open(string("/proc/" + pid + "/cmdline").c_str());
        app >> Topapp;
        app.close();
        
        while (Topapp[Topapp.length() - 1] == '\0') // trim
           Topapp.pop_back();
    }
    else
    {
        Shell("dumpsys activity activities|grep topResumedActivity=|tail -n 1|cut -d \"{\" -f2|cut -d \"/\" -f1|cut -d \" \" -f3", Topapp);
    }
}
static void Topappmonitor (string &Topapp) {
    prctl(PR_SET_NAME, "Topappmonitor");
    
    DAEMON: // 检测前台包名的守护进程
    Topapp = getTopapp();
    
    sleep(1)
    
    goto DAEMON;
}

void startTopappmonitor (string &Topapp) {
    std::thread t_Topappmonitor (Topappmonitor, std::ref(Topapp));
    t_Topappmonitor.detach();
}