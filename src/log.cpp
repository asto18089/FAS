#include <ctime>
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

template <typename T>
void Log::write(LogLevel level, const T& message) {
   if (level >= currentLevel) { // 如果要写入的级别大于等于当前级别，则输出到文件或控制台

       time_t now = time(nullptr); // 获取当前时间戳
       char buffer[20]; 
       strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&now)); // 格式化时间字符串

       file << "[" << buffer << "] [" << levelToString(level) << "] " << message << "\n"; // 写入到文件

       if (level == LogLevel::Error || level == LogLevel::Warning) { // 如果是错误或警告信息，则也输出到控制台
           std::cerr << "[" << buffer << "] [" << levelToString(level) << "] " << message << "\n";
       }
   }
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