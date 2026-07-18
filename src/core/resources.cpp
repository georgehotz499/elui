#include "resources.h"


// 根目录
#ifdef _WIN64
#define RES_ROOT "./res/"
#elif __linux__
#define RES_ROOT "/res/"
#endif

// data目录
#ifdef _WIN64
#define DATA "./data/"
#elif __linux__
#define DATA "/data/"
#endif

// 字体目录
#ifdef _WIN64
//#define FONT RES_ROOT"ttf/2312_v9.ttf"
#define FONT RES_ROOT"ttf/2312_v9.ttf"
#elif __linux__
#define FONT RES_ROOT"ttf/2312_v9.ttf"
#endif

std::string Resources::GetResourcecsPath() {
    return RES_ROOT;
}

std::string Resources::GetDataPath() {
    return DATA;
}

std::string Resources::GetFontPath() {
    return FONT;
}
