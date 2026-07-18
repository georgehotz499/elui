#pragma once
#include "widget.h"

#include "core/size.h"

extern "C" {
// giflib库
#include "gif_lib.h"
}

#include <string>
#include <vector>


class Gif : public Widget {
private:
    // 帧数据信息
    struct FrameInfo {
        // 帧延时，单位ms
        int m_delay{ 100 };
        // 帧图像数据指针struct lv_image_dsc_t
        void* m_image_dsc{ nullptr };
    };

public:
    Gif(std::string name, Widget* parent);
    ~Gif();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
    * @brief 获取gif文件宽高
    * @param file_path 文件路径
    * @return 返回图像大小；值为(0,0)时代表获取失败
    */
    static Size GetGifSize(const std::string& file_path);

    /**
     * @brief 设置gif图片
     * @param file_path 文件路径
     */
    void SetImage(const std::string& file_path);

private:
    /**
     * @brief 解析gif全部帧
     * @param file_path 文件路径
     * @return true gif数据解析成功；false 解析gif失败
     */
    bool LoadGif(const std::string& file_path);

    /**
     * @brief 解析gif帧数据
     * @param gif_file gif文件句柄
     * @return true 解析成功；false 解析失败
     */
    bool DecodeFrames(GifFileType* gif_file);

    /**
     * @brief 获取帧控制信息
     * @param gif_file gif文件句柄
     * @param index 帧索引
     * @param delay 帧延时，单位ms
     * @param transparent_idx 透明度索引
     * @param disposal_mode 帧处置方式
     */
    void GetFrameControl(GifFileType* gif_file, int index, int& delay, int& transparent_idx, int& disposal_mode);

    /**
     * @brief 绘制gif帧到画布
     * @param gif_file gif文件句柄
     * @param frame 帧数据
     * @param transparent_idx 透明度索引
     * @param canvas 画布数据
     * @return true 绘制成功；false 绘制失败
     */
    bool DrawFrame(GifFileType* gif_file, const SavedImage& frame, int transparent_idx, std::vector<uint8_t>& canvas);

    /**
     * @brief 保存当前画布为帧图像
     * @param canvas 画布数据
     * @param delay 帧延时，单位ms
     * @param frame_info 帧图像数据
     * @return true 保存成功；false 保存失败
     */
    bool SaveFrameImage(const std::vector<uint8_t>& canvas, int delay, FrameInfo& frame_info);

    /**
     * @brief 清空帧区域
     * @param canvas 画布数据
     * @param frame_desc 帧描述信息
     */
    void ClearFrameArea(std::vector<uint8_t>& canvas, const GifImageDesc& frame_desc);

    /**
    * @brief 刷新一帧图像数据
    */
    void UpdateImage();

    /**
    * @brief 启动定时器，刷新图像
    * @param delay 延时时间（单位：ms）
    */
    void StartTimerToUpdateImage(int delay);

    /**
    * @brief 停止图像刷新定时器
    */
    void StopTimerToUpdateImage();

    /**
     * @brief 释放帧图像缓存
     */
    void ReleaseImageCache();

    /**
     * @brief 执行定时器回调
     * @param timer_id 定时器id
     */
    void ExecuteTimerCallback(int timer_id) override;

private:
    // gif图片路径
    std::string m_image_path;
    // gif图像宽度
    int m_width{ 0 };
    // gif图像高度
    int m_height{ 0 };
    // gif帧索引
    int m_frame_index{ 0 };
    // gif帧图像缓存
    std::vector<FrameInfo> m_frame_list;
};
