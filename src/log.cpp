#include <ctime>
#include <iostream>

#include "include/log.h"

void Log::write(LogLevel level, const std::string &message)
{
    if (level >= currentLevel)
    {
        const time_t now = time(nullptr);
        char buffer[20] = {'\0'};
        strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
        
        std::string log_message = '[' + std::string(buffer) + "] [" + levelToString(level) + "] " + message + '\n';
        int fd = open(DF_LOGFILE, O_WRONLY | O_APPEND | O_CREAT);
        if (fd != -1)
        {
            ::write(fd, log_message.c_str(), log_message.size());
            close(fd);
        }
        std::cout << log_message;
    }
}

const char *Log::levelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    default:
        return "";
    }
}