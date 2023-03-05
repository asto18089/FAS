#pragma once

#include <iostream>
#include <fstream>
#include <string>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Log {
public:
    Log(const std::string& filename);
    ~Log();
    void setLevel(LogLevel level);
    void write(LogLevel level, const std::string& message);
private:
    std::ofstream file;
    LogLevel currentLevel;
    std::string levelToString(LogLevel level);
};
