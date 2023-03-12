#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <string>

#define DF_LOGLEVEL 1
#define DF_LOGFILE "/storage/emulated/0/Android/FAS/FasLog.txt"

#if DF_LOGLEVEL == 0
#define LOGLEVEL LogLevel::Debug
#define DEBUG(x) Log::write(LogLevel::Debug, x)
#define INFO(x) Log::write(LogLevel::Info, x)
#define WARN(x) Log::write(LogLevel::Warning, x)
#define ERROR(x) Log::write(LogLevel::Error, x)
#elif DF_LOGLEVEL == 1
#define LOGLEVEL LogLevel::Info
#define DEBUG(x) 
#define INFO(x) Log::write(LogLevel::Info, x)
#define WARN(x) Log::write(LogLevel::Warning, x)
#define ERROR(x) Log::write(LogLevel::Error, x)
#elif DF_LOGLEVEL == 2
#define LOGLEVEL LogLevel::Warning
#define DEBUG(x) 
#define INFO(x) 
#define WARN(x) Log::write(LogLevel::Warning, x)
#define ERROR(x) Log::write(LogLevel::Error, x)
#elif DF_LOGLEVEL == 3
#define LOGLEVEL LogLevel::Error
#define DEBUG(x) 
#define INFO(x) 
#define WARN(x) 
#define ERROR(x) Log::write(LogLevel::Error, x)
#endif

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
    static void write(LogLevel level, const std::string& message);
private:
    static const LogLevel currentLevel = LOGLEVEL;
    static const char *levelToString(LogLevel level);
};