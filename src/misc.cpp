#include "include/misc.h"

#include <spawn.h>
#include <iostream>
#include <boost/process.hpp>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>

namespace bp = boost::process;

std::string execCmdSync(const std::string &command, const std::vector<std::string> &args) {
    bp::ipstream out, err;
    std::error_code ec; // error code
    bp::child c(command, args, bp::std_out > out, bp::std_err > err, ec);
    if (ec) // check for errors
    {
        std::cerr << "Error: " << ec.message() << '\n';
        return {};
    }
    
    std::string result, line;
    while (out && std::getline(out, line))
        result += line + '\n';
    while (err && std::getline(err, line))
        result += line + '\n';
        
    c.wait();

    return result;
}

void bound2little()
{
    cpu_set_t mask;
    CPU_ZERO(&mask);

    std::ifstream fd("/sys/devices/system/cpu/cpufreq/policy0/related_cpus");
    std::string related_cpus;
    std::getline(fd, related_cpus);
    std::istringstream cut(related_cpus);

    int cpu = 0;
    while (cut >> cpu)
        CPU_SET(cpu, &mask);

    sched_setaffinity(0, sizeof(mask), &mask);
}

std::string getTopApp()
{
    std::string name;
    auto Testfile = [](const char *location) { return access(location, F_OK) == 0; };
    
    if (Testfile("/sys/kernel/gbe/gbe2_fg_pid"))
    {
        std::string pid;
        std::ifstream f_pid, app;
        f_pid.open("/sys/kernel/gbe/gbe2_fg_pid");
        if (!f_pid)
            return {};
        f_pid >> pid;

        app.open("/proc/" + pid + "/cmdline");
        if (!app)
            return {};
        std::getline(app, name, '\0');

        return name;
    }
    const std::string str = execCmdSync("/system/bin/dumpsys", {"window", "visible-apps"});
        
    const auto pkgPos = str.find("package=") + 8;

    name = str.substr(pkgPos, str.find(' ', pkgPos) - pkgPos);
    return name;
}