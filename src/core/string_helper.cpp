#include "string_helper.h"

#include <sstream>

std::string StringHelper::ToUpperCase(std::string str) {
    std::string result;
    result.reserve(str.size());  // 预分配内存，提高效率
    for (char c : str) {
        // 注意：toupper() 要求输入为 unsigned char 或 EOF，否则可能有未定义行为
        result += static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string StringHelper::ToLowerCase(std::string str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
        result += static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string StringHelper::ToString(long long num) {
    return std::to_string(num);
}

std::string StringHelper::ToString(int num) {
    return std::to_string(num);
}

std::string StringHelper::ToString(char num) {
    return std::to_string(num);
}

std::string StringHelper::ToString(float num) {
    return std::to_string(num);
}

std::string StringHelper::ToString(double num) {
    return std::to_string(num);
}

std::string StringHelper::ToString(const std::thread::id& num) {
    std::stringstream ss;
    ss << num;  // 利用 << 运算符转换
    return ss.str();
}

long long StringHelper::ToLongLong(const std::string& str) {
    return std::stoll(str);
}

int StringHelper::ToInt(const std::string& str) {
    return std::stoi(str);
}

float StringHelper::ToFloat(const std::string& str) {
    return std::stof(str);
}

double StringHelper::ToDouble(const std::string& str) {
    return std::stod(str);
}

std::vector<std::string> StringHelper::SplitString(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    size_t start = 0;          // 起始位置
    size_t pos = 0;            // 分隔符位置
    size_t delimiter_len = delimiter.length();

    // 空分隔符直接返回原字符串
    if (delimiter_len == 0) {
        result.push_back(str);
        return result;
    }

    // 循环查找分隔符
    while ((pos = str.find(delimiter, start)) != std::string::npos) {
        // 截取 [start, pos) 区间的子串
        result.push_back(str.substr(start, pos - start));
        // 更新起始位置为分隔符结束位置
        start = pos + delimiter_len;
    }

    // 处理最后一段
    result.push_back(str.substr(start));

    return result;
}

std::string StringHelper::RemoveSubstring(std::string str, const std::string& subStr) {
    if (subStr.empty()) {
        return str;
    }
    // 1. 查找子串位置
    size_t pos = str.find(subStr);
    if (pos != std::string::npos) { // 找到子串才删除
        // 2. 从pos位置开始，删除subStr长度的字符
        str.erase(pos, subStr.length());
    }
    return str;
}

std::string StringHelper::RemoveAllSubstrings(std::string str, const std::string& subStr) {
    if (subStr.empty()) {
        return str;
    }
    size_t pos = 0;
    while ((pos = str.find(subStr, pos)) != std::string::npos) {
        str.erase(pos, subStr.length());
        // 无需pos++，因为删除后后续字符会前移，从当前pos继续查找即可
    }
    return str;
}

std::string StringHelper::Replace(std::string sou, const std::string& old, const std::string& des) {
    size_t pos = 0;
    // 循环查找并替换所有匹配的子串
    while ((pos = sou.find(old, pos)) != std::string::npos) {
        sou.replace(pos, old.length(), des);
        pos += des.length();  // 移动指针，避免重复替换
    }
    return sou;
}

std::string StringHelper::ToHex(const std::string& sou) {
    if (sou.empty()) { return ""; }
    std::string tmp;
    char buf[32]{ 0 };
    for (char iter : sou) {
        snprintf(buf, sizeof(buf), "%02X ", (uint8_t)iter);
        tmp.append(buf);
    }
    return tmp;
}

std::string StringHelper::GetFileName(const std::string& path) {
    // 查找最后一个 '/' 的位置
    size_t last_slash_pos = path.find_last_of('/');

    // 如果没找到 '/'，说明路径本身就是文件名；否则截取 '/' 之后的部分
    if (last_slash_pos == std::string::npos) {
        return path;
    }
    return path.substr(last_slash_pos + 1);
}
