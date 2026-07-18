#pragma once
#include <string>

class Resources {
public:
    /**
    * @brief 获取资源目录
    */
    static std::string GetResourcecsPath();

    /**
    * @brief 获取data目录
    */
    static std::string GetDataPath();

    /**
    * @brief 获取字体全路径
    */
    static std::string GetFontPath();
};
