#include "log.h"
// 添加宏，避免在msvc环境下报错
#define _CRT_SECURE_NO_WARNINGS
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <ctime>


static std::mutex mutex_;

static std::string GetTime() {
    // 获取当前高精度时间点
    auto now = std::chrono::system_clock::now();

    // 提取秒级时间，转换为本地时间
    std::time_t sec_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&sec_time);
    if (local_time == nullptr) {
        std::cerr << "Failed to get local time." << std::endl;
        return "";
    }

    // 计算毫秒（当前时间点 - 秒级时间点 = 毫秒部分）
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - std::chrono::system_clock::from_time_t(sec_time)
    );
    char time_str[32] = {0};
    snprintf(time_str, sizeof(time_str), "%.2d:%.2d:%.2d.%.3d", local_time->tm_hour, local_time->tm_min, local_time->tm_sec, ms.count());

    return time_str;
}

static std::string FormatString(const char* format, va_list args) {
    char buffer[1024];
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(buffer, sizeof(buffer), format, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        return "Formatting error";
    }

    if (needed < static_cast<int>(sizeof(buffer))) {
        return std::string(buffer, needed);
    }

    std::vector<char> dynamicBuffer(needed + 1);
    va_copy(args_copy, args);
    vsnprintf(dynamicBuffer.data(), dynamicBuffer.size(), format, args_copy);
    va_end(args_copy);

    return std::string(dynamicBuffer.data(), needed);
}

static void Flush(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    // 这里可以扩展为写入文件或其他目标
    std::cout << message << std::endl;
}

void LogOut(const char* file, const int line, LogLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = FormatString(format, args);
    va_end(args);

    std::string concat;
    concat.append(GetTime());
    concat.append(" ");
    concat.append(kDebug == level ? "D " : kInfo == level ? "I " : kWarn == level ? "W " : kError == level ? "E " : "Unknown");
    concat.append(" ");
    concat.append(file);
    concat.append(" ");
    concat.append(std::to_string(line));
    concat.append(" ");
    concat.append(message);

    Flush(concat);
}
