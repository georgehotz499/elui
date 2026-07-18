#pragma once
#include "widget/object.h"

#include <string>
#include <unordered_set>


class FileHelper {
public:
    /**
    * @brief 读取文件
    * @param path 文件路径
    * @return 返回文件数据,返回数据为空代表可能文件不存在读取失败
    */
    static std::string ReadFile(const std::string& path);
    static void ReadFile(const std::string& path, std::string& out_file);

    /**
    * @brief 写入文件
    * @param content 数据内容
    * @param path 文件保存路径
    */
    static void WriteFile(const std::string& content, const std::string& path);

    /**
    * @brief 创建文件目录
    * @param dir 目录名称
    */
    static void CreatDir(std::string dir);

    /**
    * @brief 删除目录
    * @param dir 目录名称
    */
    static void DeleteDir(std::string dir);

    /**
    * @brief 判断文件是否存在
    * @param file_path 文件路径
    * @return true 文件存在；false 文件不存在
    */
    static bool CheckFileExist(const std::string& file_path);

    /**
    * @brief 判断目录是否存在
    * @param path 目录路径
    * @return true 目录存在；false 目录不存在
    */
    static bool CheckDirecroyExist(const std::string& path);

    /**
    * @brief 获取目录下所有文件
    * @param path 目录路径
    * @return 返回读取到的文件全路径
    */
    static std::unordered_set<std::string> GetFilesInDir(const std::string& dirPath);

    /**
    * @brief 删除一个文件
    * @param file_path 文件路径
    * @return true 删除成功；false 删除失败
    */
    static bool DelFile(const std::string& file_path);
};
