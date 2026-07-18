#pragma once
#include <string>
#include <map>
#include <set>


// 图片信息描述符
using ImageDes = void*;

class ImageIns {
public:
    /**
     * @brief 解析图片数据
     * @param image_path 文件路径
     * @return 返回图片数据指针，nullptr 解析图片失败
     */
    static ImageDes ParseImage(const std::string& image_path);

    /**
    * @brief 析构ImageDes指针
    */
    static void DestroyImageDes(ImageDes des);

    /**
     * @brief 缓存图片
     * @param image_path 文件路径
     * @return 返回图片数据指针，nullptr 解析图片失败
     */
    static void CacheImage(const std::string& image_path);

    /**
     * @brief 移除缓存图片
     * @param image_path 文件路径
     */
    static void RemoveCacheImage(const std::string& image_path);

    /**
     * @brief 获取缓存的图片数量
     */
    static int GetImageCount();

private:
    // 保存预加载图片
    static std::map<std::string, ImageDes> m_cache;
    // 标记已缓存图片
    static std::set<ImageDes>  m_image_cache;
};
