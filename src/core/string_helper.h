#pragma once
#include <string>
#include <thread>
#include <vector>

class StringHelper {
public:
	/**
	* @brief 字符串转大写
	* @param str 需要转换的字符串
	* @return 返回转换好的字符串
	*/
	static std::string ToUpperCase(std::string str);

	/**
	* @brief 字符串转小写
	* @param str 需要转换的字符串
	* @return 返回转换好的字符串
	*/
	static std::string ToLowerCase(std::string str);

	/**
	* @brief long long转为string
	*/
	static std::string ToString(long long num);

    /**
    * @brief int转为string
    */
    static std::string ToString(int num);

    /**
    * @brief char 转为16进制字符串
    */
    static std::string ToString(char num);

    /**
    * @brief float 转为string
    */
    static std::string ToString(float num);

    /**
    * @brief double 转为string
    */
    static std::string ToString(double num);

    /**
    * @brief std::thread::id转为string
    */
    static std::string ToString(const std::thread::id& num);

	/**
	* @brief string转为long long
	*/
	static long long ToLongLong(const std::string& str);

    /**
    * @brief string转为int
    */
    static int ToInt(const std::string& str);

    /**
    * @brief string转为float
    */
    static float ToFloat(const std::string& str);

    /**
    * @brief string转为double
    */
    static double ToDouble(const std::string& str);

    /**
     * @brief 按任意字符串分隔符分割字符串（支持单/多字符分隔符）
     * @param str 待分割的字符串
     * @param delimiter 分隔符（任意字符串）
     * @return 分割后的字符串列表
     */
    static std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter);

    /**
    * @brief 移除字符串中第一个匹配的子串
    * @param str 源字符串
    * @param sub_str 移除的字符串
    * @return 返回处理后的字符串
    */
    static std::string RemoveSubstring(std::string str, const std::string& subStr);

    /**
    * @brief 移除字符串中所有匹配的子串（如需批量删除）
    * @param str 源字符串
    * @param sub_str 移除的字符串
    * @return 返回处理后的字符串
    */
    static std::string RemoveAllSubstrings(std::string str, const std::string& subStr);

    /**
    * @brief 替换指定字符串为新的字串
    * @param sou 源字符串
    * @param old 被替换的字符串
    * @param des 用于替换的字符串
    * @return 返回替换后的字符串
    */
    static std::string Replace(std::string sou, const std::string& old, const std::string& des);

    /**
    * @brief 将string对象转为16进制
    * @param sou 源字符串
    * @return 返回转换后的字符串
    */
    static std::string ToHex(const std::string& sou);

    /**
    * @brief 从路径中获取文件名
    * @param path 文件路径
    * @return 返回文件名称
    */
    static std::string GetFileName(const std::string& path);
};
