#pragma once

#include <iostream>
#include <fstream>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Log {
public:
    Log(const std::string& filename);
    void setLevel(LogLevel level);
    void write(LogLevel level, const char* message);
private:
    std::ofstream file;
    LogLevel currentLevel;
    static const char* levelToString(LogLevel level);
};
