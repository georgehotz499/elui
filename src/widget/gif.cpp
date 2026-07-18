#include "gif.h"

#include "core/file_helper.h"
#include "core/log.h"

extern "C" {
#include "lvgl/lvgl.h"
}

#include <algorithm>
#include <cstring>


// 刷新图像
#define UPDATE_IMAGE 0
// 默认帧延时
#define DEFAULT_FRAME_DELAY 100
// ARGB图像像素长度
#define ARGB_PIXEL_LEN 4

Gif::Gif(std::string name, Widget* parent) :Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());
}

Gif::~Gif() {
    // 释放解析后的帧图像缓存
    ReleaseImageCache();
}

void Gif::Create(ScreenLayer layer) {
    m_object = lv_image_create(layer);
}

Size Gif::GetGifSize(const std::string& file_path) {
    Size size;
    int error_code{ 0 };
    // 打开GIF文件
    GifFileType* gif_file = DGifOpenFileName(file_path.c_str(), &error_code);
    if (gif_file == nullptr) {
        LOGE("Failed to open GIF file:%s error_code:%d", file_path.c_str(), error_code);
        return size;
    }

    // 将整个GIF数据读取到内存中
    if (DGifSlurp(gif_file) == GIF_ERROR) {
        LOGE("Error: Failed to read GIF data. Error code:%d", gif_file->Error);
        DGifCloseFile(gif_file, nullptr);
        return size;
    }

    size.m_width = gif_file->SWidth;
    size.m_height = gif_file->SHeight;

    // 释放资源
    DGifCloseFile(gif_file, nullptr);

    return size;
}

void Gif::SetImage(const std::string& file_path) {
    // 重新设置图片前释放旧图片缓存
    ReleaseImageCache();

    // 文件不存在直接返回
    if (!FileHelper::CheckFileExist(file_path)) {
        LOGE("File not exist:%s", file_path.c_str());
        return;
    }

    // 一次性解析全部GIF帧数据
    if (!LoadGif(file_path)) {
        return;
    }

    m_image_path = file_path;
    m_frame_index = 0;

    // 未设置控件大小时，使用gif原始宽高
    Rect rect = GetGeometry();
    if (0 == rect.m_width || 0 == rect.m_height) {
        Widget::SetSize(m_width, m_height);
    }

    // 刷新第一帧图像数据
    UpdateImage();
}

bool Gif::LoadGif(const std::string& file_path) {
    int error_code{ 0 };
    // 打开GIF文件
    GifFileType* gif_file = DGifOpenFileName(file_path.c_str(), &error_code);
    if (gif_file == nullptr) {
        LOGE("Failed to open GIF file:%s error_code:%d", file_path.c_str(), error_code);
        return false;
    }

    // 将整个GIF数据读取到内存中
    if (DGifSlurp(gif_file) == GIF_ERROR) {
        LOGE("Failed to read GIF data. Error code:%d", gif_file->Error);
        DGifCloseFile(gif_file, nullptr);
        return false;
    }

    const bool result = DecodeFrames(gif_file);

    // gif文件已解析成帧缓存，文件句柄可直接释放
    DGifCloseFile(gif_file, nullptr);

    if (!result) {
        ReleaseImageCache();
    }
    return result;
}

bool Gif::DecodeFrames(GifFileType* gif_file) {
    if (!gif_file || gif_file->SWidth <= 0 || gif_file->SHeight <= 0 || gif_file->ImageCount <= 0) {
        LOGE("Invalid gif data.");
        return false;
    }

    m_width = gif_file->SWidth;
    m_height = gif_file->SHeight;

    // 当前画布数据，每一帧都基于gif处置规则合成到此画布上
    std::vector<uint8_t> canvas(m_width * m_height * ARGB_PIXEL_LEN, 0X00);

    for (int index = 0; index < gif_file->ImageCount; ++index) {
        SavedImage& frame = gif_file->SavedImages[index];

        // 获取帧控制信息
        int delay = DEFAULT_FRAME_DELAY;
        int transparent_idx = NO_TRANSPARENT_COLOR;
        int disposal_mode = DISPOSAL_UNSPECIFIED;
        GetFrameControl(gif_file, index, delay, transparent_idx, disposal_mode);

        // DISPOSE_PREVIOUS需要在绘制当前帧前保存画布
        std::vector<uint8_t> previous_canvas;
        if (DISPOSE_PREVIOUS == disposal_mode) {
            previous_canvas = canvas;
        }

        if (!DrawFrame(gif_file, frame, transparent_idx, canvas)) {
            LOGE("Failed to draw gif frame:%d", index);
            return false;
        }

        FrameInfo frame_info;
        if (!SaveFrameImage(canvas, delay, frame_info)) {
            LOGE("Failed to save gif frame:%d", index);
            return false;
        }
        m_frame_list.push_back(frame_info);

        // 当前帧显示完毕后，按处置方式准备下一帧画布
        switch (disposal_mode) {
        case DISPOSE_BACKGROUND:
            ClearFrameArea(canvas, frame.ImageDesc);
            break;
        case DISPOSE_PREVIOUS:
            if (!previous_canvas.empty()) {
                canvas = previous_canvas;
            }
            break;
        case DISPOSE_DO_NOT:
        case DISPOSAL_UNSPECIFIED:
        default:
            break;
        }
    }

    return !m_frame_list.empty();
}

void Gif::GetFrameControl(GifFileType* gif_file, int index, int& delay, int& transparent_idx, int& disposal_mode) {
    // 默认延迟：100ms，符合GIF常见默认播放间隔
    delay = DEFAULT_FRAME_DELAY;
    transparent_idx = NO_TRANSPARENT_COLOR;
    disposal_mode = DISPOSAL_UNSPECIFIED;

    GraphicsControlBlock gcb;
    std::memset(&gcb, 0, sizeof(GraphicsControlBlock));
    gcb.TransparentColor = NO_TRANSPARENT_COLOR;
    gcb.DelayTime = DEFAULT_FRAME_DELAY / 10;
    gcb.DisposalMode = DISPOSAL_UNSPECIFIED;

    if (DGifSavedExtensionToGCB(gif_file, index, &gcb) == GIF_ERROR) {
        return;
    }

    // gif帧延时单位为1/100秒，转为ms
    delay = (gcb.DelayTime > 0) ? (gcb.DelayTime * 10) : DEFAULT_FRAME_DELAY;
    transparent_idx = gcb.TransparentColor;
    disposal_mode = gcb.DisposalMode;
}

bool Gif::DrawFrame(GifFileType* gif_file, const SavedImage& frame, int transparent_idx, std::vector<uint8_t>& canvas) {
    if (!gif_file || !frame.RasterBits) {
        return false;
    }

    const GifImageDesc& frame_desc = frame.ImageDesc;
    // 优先使用帧局部颜色表，无则使用全局颜色表
    ColorMapObject* color_map = frame_desc.ColorMap ? frame_desc.ColorMap : gif_file->SColorMap;
    if (!color_map) {
        LOGE("No color map found for frame.");
        return false;
    }

    for (int y = 0; y < frame_desc.Height; ++y) {
        for (int x = 0; x < frame_desc.Width; ++x) {
            const int src_index = y * frame_desc.Width + x;
            const GifByteType color_idx = frame.RasterBits[src_index];

            // 透明像素不覆盖画布原有数据
            if (transparent_idx != NO_TRANSPARENT_COLOR && color_idx == static_cast<GifByteType>(transparent_idx)) {
                continue;
            }
            if (color_idx >= color_map->ColorCount) {
                continue;
            }

            const int dest_x = frame_desc.Left + x;
            const int dest_y = frame_desc.Top + y;
            if (dest_x < 0 || dest_x >= m_width || dest_y < 0 || dest_y >= m_height) {
                continue;
            }

            const GifColorType& color = color_map->Colors[color_idx];
            const int dest_index = (dest_y * m_width + dest_x) * ARGB_PIXEL_LEN;

            // LVGL ARGB8888内存顺序按BGRA写入
            canvas[dest_index] = color.Blue;
            canvas[dest_index + 1] = color.Green;
            canvas[dest_index + 2] = color.Red;
            canvas[dest_index + 3] = 0XFF;
        }
    }

    return true;
}

bool Gif::SaveFrameImage(const std::vector<uint8_t>& canvas, int delay, FrameInfo& frame_info) {
    if (canvas.empty() || m_width <= 0 || m_height <= 0) {
        return false;
    }

    const int image_size = m_width * m_height * ARGB_PIXEL_LEN;

    // 创建图像存储空间
    uint8_t* image_data = new uint8_t[image_size];
    std::memcpy(image_data, canvas.data(), image_size);

    lv_image_dsc_t* dsc = new lv_image_dsc_t;
    std::memset(dsc, 0, sizeof(lv_image_dsc_t));
    dsc->header.cf = LV_COLOR_FORMAT_ARGB8888;
    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.w = m_width;
    dsc->header.h = m_height;
    dsc->header.stride = m_width * ARGB_PIXEL_LEN;
    dsc->data_size = image_size;
    dsc->data = image_data;

    frame_info.m_delay = delay;
    frame_info.m_image_dsc = dsc;

    return true;
}

void Gif::ClearFrameArea(std::vector<uint8_t>& canvas, const GifImageDesc& frame_desc) {
    const int frame_left = static_cast<int>(frame_desc.Left);
    const int frame_top = static_cast<int>(frame_desc.Top);
    const int frame_right = static_cast<int>(frame_desc.Left + frame_desc.Width);
    const int frame_bottom = static_cast<int>(frame_desc.Top + frame_desc.Height);

    const int left = (frame_left < 0) ? 0 : frame_left;
    const int top = (frame_top < 0) ? 0 : frame_top;
    const int right = (frame_right > m_width) ? m_width : frame_right;
    const int bottom = (frame_bottom > m_height) ? m_height : frame_bottom;

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            const int index = (y * m_width + x) * ARGB_PIXEL_LEN;
            canvas[index] = 0X00;
            canvas[index + 1] = 0X00;
            canvas[index + 2] = 0X00;
            canvas[index + 3] = 0X00;
        }
    }
}

void Gif::UpdateImage() {
    if (m_frame_list.empty()) {
        LOGE("Gif frame list is empty.");
        return;
    }

    const int frame_count = static_cast<int>(m_frame_list.size());
    m_frame_index = m_frame_index % frame_count;
    FrameInfo& frame_info = m_frame_list[m_frame_index];

    // 设置图像资源
    lv_image_set_src(m_object, static_cast<lv_image_dsc_t*>(frame_info.m_image_dsc));

    // 帧索引指向下一帧
    m_frame_index = (m_frame_index + 1) % frame_count;

    // 多帧gif启动定时器继续刷新
    if (frame_count > 1) {
        StartTimerToUpdateImage(frame_info.m_delay);
    }
}

void Gif::StartTimerToUpdateImage(int delay) {
    StopTimerToUpdateImage();
    StartTimer(UPDATE_IMAGE, delay);
}

void Gif::StopTimerToUpdateImage() {
    if (m_timers.end() != m_timers.find(UPDATE_IMAGE)) {
        StopTimer(UPDATE_IMAGE);
    }
}

void Gif::ReleaseImageCache() {
    StopTimerToUpdateImage();

    for (auto& frame_info : m_frame_list) {
        lv_image_dsc_t* dsc = static_cast<lv_image_dsc_t*>(frame_info.m_image_dsc);
        if (!dsc) {
            continue;
        }

        // 释放图像数据
        delete[] const_cast<uint8_t*>(dsc->data);
        // 释放lv_image_dsc_t结构体
        delete dsc;
        frame_info.m_image_dsc = nullptr;
    }

    m_frame_list.clear();
    m_frame_index = 0;
    m_width = 0;
    m_height = 0;
    m_image_path.clear();

    if (m_object) {
        lv_image_set_src(m_object, nullptr);
    }
}

void Gif::ExecuteTimerCallback(int timer_id) {
    switch (timer_id) {
    // 刷新图像
    case UPDATE_IMAGE:
        UpdateImage();
        break;
    default:
        LOGW("Unsolved timer %d", timer_id);
        break;
    }
}
