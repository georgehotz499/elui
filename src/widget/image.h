#pragma once
#include "widget.h"


class Image : public Widget {
public:
    // 图片类型
    using ImageType = enum { kTypeNone = 0, kTypeJpeg, kTypePng };
    // 挖空图像数据存储类型
    struct ImageClip {
        // 图像数据存储
        void* m_image_dsc{ nullptr };
        // 图像显示区域
        Rect m_Image_area;
    };
    // 图片挖空数据存储
    using ImageVec = std::list<ImageClip>;

public:
    Image(std::string name, Widget* parent);
    ~Image();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

    /**
     * @brief 释放加载数据
     */
    void ReleaseImageDsc();

public:
    /**
    * @brief 设置图片
    * @param image_path 图片路径
    */
    void SetImage(const std::string& image_path);

    /**
    * @brief 设置图片
    * @param width 图片宽度
    * @param height 图片高度
    * @param image_arr 经过官方转换工具取模的图像数组（工具链接：https://lvgl.io/tools/imageconverter）
    * @param type 图片类型
    */
    void SetImage(int width, int height, uint8_t* image_arr, Image::ImageType type = kTypeJpeg);

    /**
     * @brief 设置挖空区域
     * @param trans_rect 挖空区域
     */
    void SetClipArea(const Rect& trans_rect);

    /**
     * @brief 移除挖空区域
     */
    void ResetClipArea();

    /**
     * @brief 获取拆分的图像数据
     */
    ImageVec GetClipImage();

protected:
    /**
    * @brief 拆分图像
     * @param trans_rect 挖空区域
    */
    void ClipImage(const Rect& trans_rect);

    /**
    * @brief 释放之前切分的图片
    */
    void RealeseClipImage();

private:
    // 图片数据指针struct lv_image_dsc_t
    void* m_image_dsc{ nullptr };
    // 图片数据是否由ImageIns创建
    bool m_image_from_image_ins{ false };
    // 图片路径
    std::string m_image_path;
    // 挖空图像存储
    ImageVec m_clip_image;
    // 挖空区域
    Rect m_clip_area;
};
