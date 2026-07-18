#pragma once

#ifdef __cplusplus  // 仅在C++编译器中生效
extern "C" {
#endif

typedef  enum _LogLevel {
    kDebug = 0,
    kInfo,
    kWarn,
    kError
} LogLevel;

void LogOut(const char* file, const int line, LogLevel level, const char* format, ...);

#define LOGD(format, ...) LogOut(__FILE__, __LINE__, kDebug, format, ##__VA_ARGS__)
#define LOGI(format, ...) LogOut(__FILE__, __LINE__, kInfo, format, ##__VA_ARGS__)
#define LOGW(format, ...) LogOut(__FILE__, __LINE__, kWarn, format, ##__VA_ARGS__)
#define LOGE(format, ...) LogOut(__FILE__, __LINE__, kError, format, ##__VA_ARGS__)

//#define DEBUG LOGD("Debug!!");
    
#ifdef __cplusplus  // 仅在C++编译器中生效
}
#endif