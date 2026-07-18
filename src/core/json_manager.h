#pragma once
#include "core/json.hpp"
using Json = nlohmann::json;

class JsonManager {
public:
    /**
    * @brief 解析json对象
    * @param json_str json的文本信息
    * @param json 出参解析后的json对象
    * @return true 解析成功；false 解析失败
    */
    static bool ParseJson(const std::string& json_str, Json& json);

    /**
    * @brief 解析数据
    * @param json 包含数据的json对象
    * @param key 解析数据的键
    * @param value 解析出参
    * @return true 解析成功；false 解析失败
    */
    static bool Parse(const Json& json, const std::string& key, std::string& value);

    static bool Parse(const Json& json, const std::string& key, Json& value);

    static bool Parse(const Json& json, const std::string& key, long long& value);

    static bool Parse(const Json& json, const std::string& key, int& value);

    static bool Parse(const Json& json, const std::string& key, bool& value);

    static bool ParseArray(const Json& json, const std::string& key, Json& value);
};
