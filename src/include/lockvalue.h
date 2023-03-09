#pragma once

#include <fstream>
#include <string>
#include <sys/stat.h>

// Edit and Lock a file
template <typename T>
static bool Lockvalue(const std::string &location, const T &value)
{
    std::ifstream test(location);
    if (! test) {return false;}
    
    T temp;
    test >> temp;
    if (temp == value) 
    {
        test.close();
        return true;
    }
    
    chmod(location.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH);
    std::ofstream fd(location, std::ios::out | std::ios::trunc);
    fd << value;
    chmod(location.c_str(), S_IRUSR | S_IRGRP | S_IROTH);
    fd.close();
    return true;
}
