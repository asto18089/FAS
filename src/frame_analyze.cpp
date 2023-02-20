#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <sys/prctl.h>
#include <chrono>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::thread;
using std::ref;
using namespace std::chrono;

static string getSurfaceview() {
    FILE *game = popen("dumpsys SurfaceFlinger --list", "r");
    
    char buffer[1024] = {0};
    
    if (game == nullptr) {
        perror("Failed");
        pclose(game);
        
        return string(); //It's empty
    }
    
    while (fgets(buffer, sizeof(buffer), game)) {
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