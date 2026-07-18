#pragma once

#include <fstream>
#include <functional>
#include <queue>
#include <string>


class Http;
struct curl_slist;

/**
 * @brief HTTP请求头信息
 */
class Header {
    friend class Http;

public:
    /**
     * @brief 添加请求头
     * @param header 请求头内容，例如 "Content-Type: application/json"
     */
    void Put(std::string header);

    /**
     * @brief 清空请求头
     */
    void Clear();

private:
    std::queue<std::string> m_headers;
};

/**
 * @brief multipart/form-data表单信息
 */
class Mime {
    friend class Http;

public:
    /**
     * @brief 表单字段信息
     */
    struct Info {
        Info() {}
        Info(const std::string& name, const std::string& data) : m_name(name), m_data(data) {}

        std::string m_name;
        std::string m_data;
    };

    /**
     * @brief 添加表单字段
     * @param info 表单字段信息
     */
    void Put(Info info);

    /**
     * @brief 清空表单字段
     */
    void Clear();

private:
    std::queue<Info> m_mimes;
};

/**
 * @brief HTTP响应信息
 */
class Response {
public:
    /**
     * @brief 响应类型
     */
    enum Type { kTypeNull, kGet, kPost, kDownload };

public:
    Response();
    Response(Type type);

    /**
     * @brief 设置响应类型
     * @param type 响应类型
     */
    void SetType(Type type);

public:
    // curl执行结果码
    int m_code{ -10086 };
    // HTTP状态码
    long m_status{ 0 };
    // 响应内容或错误信息
    std::string m_msg;
    // 响应类型
    Type m_type{ kTypeNull };
    // 下载文件输出流
    std::ofstream* m_outfile{ nullptr };
};

/**
 * @brief HTTP客户端工具
 */
class Http {
public:
    /**
     * @brief 下载进度回调
     * @param dltotal 下载总大小
     * @param dlnow 当前已下载大小
     * @return true继续下载，false停止下载
     */
    using DownloadCallback = std::function<bool(double dltotal, double dlnow)>;

public:
    /**
     * @brief 初始化curl全局环境
     */
    static void GlobalInit();

    /**
     * @brief 清理curl全局环境
     */
    static void GlobalCleanup();

    /**
     * @brief 获取秒级时间戳
     * @return 秒级时间戳
     */
    static long long GetSecondTimestamp();

    /**
     * @brief 获取毫秒级时间戳
     * @return 毫秒级时间戳
     */
    static long long GetMillisecondTimestamp();

    /**
     * @brief 计算字符串MD5
     * @param str 待计算字符串
     * @return MD5字符串，失败返回空字符串
     */
    static std::string Md5Checksum(const std::string str);

    /**
     * @brief 校验并计算文件MD5
     * @param file_path 文件路径
     * @param md5_hash 输出MD5字符串
     * @return true成功，false失败
     */
    static bool Md5CheckFile(const std::string& file_path, std::string& md5_hash);

    /**
     * @brief URL编码
     * @param origin 原始字符串
     * @return 编码后的字符串
     */
    static std::string UrlEncode(const std::string& origin);

    /**
     * @brief URL解码
     * @param origin 编码后的字符串
     * @return 解码后的字符串
     */
    static std::string UrlDecode(const std::string& origin);

    /**
     * @brief GET请求
     * @param url 请求地址
     * @param timeout 请求超时时间，单位秒；0表示不设置超时
     * @return HTTP响应信息
     */
    Response Get(const std::string url, int timeout = 0);

    /**
     * @brief 带请求头的GET请求
     * @param url 请求地址
     * @param header 请求头信息
     * @param timeout 请求超时时间，单位秒；0表示不设置超时
     * @return HTTP响应信息
     */
    Response Get(const std::string url, Header& header, int timeout = 0);

    /**
     * @brief POST请求
     * @param url 请求地址
     * @param timeout 请求超时时间，单位秒；0表示不设置超时
     * @return HTTP响应信息
     */
    Response Post(const std::string url, int timeout = 0);

    /**
     * @brief 带请求头的POST请求
     * @param url 请求地址
     * @param header 请求头信息
     * @param timeout 请求超时时间，单位秒；0表示不设置超时
     * @return HTTP响应信息
     */
    Response Post(const std::string url, Header& header, int timeout = 0);

    /**
     * @brief 带请求体的POST请求
     * @param url 请求地址
     * @param body 请求体
     * @param timeout 请求超时时间，单位秒；0表示不设置超时
     * @return HTTP响应信息
     */
    Response Post(const std::string url, const std::string& body, int timeout = 0);

    /**
     * @brief 带请求头和请求体的POST请求
     * @param url 请求地址
     * @param header 请求头信息
     * @param body 请求体
     * @param timeout 请求超时时间，单位秒；0表示不设置超时
     * @return HTTP响应信息
     */
    Response Post(const std::string url, Header& header, const std::string& body, int timeout = 0);

    /**
     * @brief 下载文件
     * @param url 下载地址
     * @param save_path 保存路径
     * @param timeout 请求超时时间，单位秒；0表示不设置超时
     * @return HTTP响应信息
     */
    Response Download(std::string url, std::string save_path, int timeout = 0);

    /**
     * @brief 上传文件
     * @param url 上传地址
     * @param mime_info 表单字段信息
     * @param file_path 文件路径
     * @param file_name 上传文件名
     * @param timeout 请求超时时间，单位秒；0表示不设置超时
     * @return HTTP响应信息
     */
    Response Upload(
        const std::string& url,
        Mime mime_info,
        const std::string& file_path,
        const std::string& file_name,
        int timeout = 0);

    /**
     * @brief 添加下载进度回调
     * @param callback 下载进度回调函数
     */
    void AddDownloadCallback(DownloadCallback callback);

    /**
     * @brief 执行下载进度回调
     * @param dltotal 下载总大小
     * @param dlnow 当前已下载大小
     * @return true继续下载，false停止下载
     */
    bool ExeDownloadCallback(double dltotal, double dlnow);

private:
    /**
     * @brief 构建curl请求头链表
     * @param header 请求头信息
     * @return curl请求头链表
     */
    static curl_slist* BuildHeaders(Header& header);

    DownloadCallback m_download_callback{ nullptr };
};
