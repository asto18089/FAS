#pragma once

#include <fstream>
#include <sys/stat.h>

// Edit and Lock a file
template <typename T>
static bool Lockvalue(const char *location, T value)
{
    chmod(location, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH);
    std::ofstream fd(location, std::ios::out | std::ios::trunc);
    if (!fd)
        return false;
    fd << value;
    chmod(location, S_IRUSR | S_IRGRP | S_IROTH);
    return true;
}
