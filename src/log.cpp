#include <ctime>
#include <iostream>
#include "include/log.h"

Log::Log(const std::string& filename) {
    file.open(filename); // 打开日志文件
}

Log::~Log() {
    file.close(); // 关闭日志文件
}

void Log::setLevel(LogLevel level) {
   currentLevel = level; // 设置当前日志级别 
}

void Log::write(LogLevel level, const std::string& message) {
   if (level >= currentLevel) {

       time_t now = time(nullptr);
       char buffer[20]; 
       strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));

       file << "[" << buffer << "] [" << levelToString(level) << "] " << message << "\n";
       std::cout << "[" << buffer << "] [" << levelToString(level) << "] " << message << "\n";
   }
   
   file.flush();
}

std::string Log::levelToString(LogLevel level) {
    switch (level) { 
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        default: return "";
   }
}