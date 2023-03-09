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
public:
    static Log &getLog(const std::string &filename)
    {
        static Log instance = Log(filename);
        return instance;
    }
    void setLevel(LogLevel level);
    void write(LogLevel level, const char *message);
private:
    Log(const std::string &filename);
    std::ofstream file;
    LogLevel currentLevel;
    static const char *levelToString(LogLevel level);
};
