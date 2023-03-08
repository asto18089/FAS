#pragma once

#include <fstream>

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

class Log
{
    Log(const std::string &filename);

public:
    static Log &getLog(const std::string &filename)
    {
        static Log instance = Log(filename);
        return instance;
    }
    void setLevel(LogLevel level);
    void write(LogLevel level, const char *message);

private:
    std::ofstream file;
    LogLevel currentLevel;
    static const char *levelToString(LogLevel level);
};
