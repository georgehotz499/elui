#pragma once
#include <vector>
#include <string>


class NetManager {
public:
    /**
     * @brief 获取IP地址
     * @return 返回IP地址集合
     */
    static std::vector<std::string> GetIpAddr();

    /**
    * @brief 获取当前WiFi连接状态
    * @return true 当前已连接；false 当前断开连接
    */
    static bool GetConnectStatus();

    /**
     * @brief 将 IPv4 字符串的最后一段替换为 255（如 "192.168.1.100" → "192.168.1.255"）
     * @param ip_str 输入的 IPv4 地址字符串（如 "192.168.0.5"）
     * @return 成功返回转换后的广播地址字符串；失败返回空字符串（格式非法）
     */
    static std::string ReplaceLastIpSegmentTo255(const std::string& ip_str);

    /**
    * @brief 获取广播地址
    * @return 返回设备ip的.255广播地址
    */
    static std::string GetBroadcastAddr();

    /**
    * @brief 获取ip地址
    * @return 返回IP地址；返回空代表获取失败
    */
    static std::string IpAddr();
};
