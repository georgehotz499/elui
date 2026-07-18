#include "image.h"

#include "core/file_helper.h"
#include "core/image_ins.h"
#include "core/log.h"
#include "core/screen.h"

extern "C" {
#include "lvgl/lvgl.h"
}

#include <cstring>


Image::Image(std::string name, Widget* parent) :Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());
}

Image::~Image() {
    // 释放加载的图片数据
    ReleaseImageDsc();
    // 释放先前切分数据
    RealeseClipImage();
}

void Image::Create(ScreenLayer layer) {
    m_object = lv_image_create(layer);
}

void Image::ReleaseImageDsc() {
    if (!m_image_dsc) {
        return;
    }

    if (m_image_from_image_ins) {
        ImageIns::DestroyImageDes(m_image_dsc);
    }
    else {
        delete static_cast<lv_image_dsc_t*>(m_image_dsc);
    }

    m_image_dsc = nullptr;
    m_image_from_image_ins = false;
}

void Image::SetImage(const std::string& image_path) {
    // 文件不存在直接返回
    if (!FileHelper::CheckFileExist(image_path)) {
        LOGE("File not exist:%s", image_path.c_str());
        return;
    }
    if (image_path == m_image_path) {
        LOGI("cur image:%s des image:%s", m_image_path.c_str(), image_path.c_str());
        return;
    }

    ImageDes image_dsc = ImageIns::ParseImage(image_path);
    if (!image_dsc) {
        LOGW("Failed to load image:%s", image_path.c_str());
        return;
    }

    // 释放旧图片并同步图片路径
    ReleaseImageDsc();
    m_image_path = image_path;
    m_image_dsc = image_dsc;
    m_image_from_image_ins = true;

    // 已设置挖空区域，则不设置图像资源
    if (Rect() != m_clip_area) {
        ClipImage(m_clip_area);
    }
    // 设置图像资源
    else {
        lv_image_set_src(m_object, static_cast<lv_image_dsc_t*>(m_image_dsc));
    }
}

void Image::SetImage(int width, int height, uint8_t* image_arr, Image::ImageType type) {
    if (m_image_from_image_ins) {
        ReleaseImageDsc();
    }
    m_image_path.clear();

    if (!m_image_dsc) {
        m_image_dsc = new lv_image_dsc_t;
    }

    lv_image_dsc_t* dsc = static_cast<lv_image_dsc_t*>(m_image_dsc);
    std::memset(dsc, 0, sizeof(lv_image_dsc_t));
    dsc->header.cf = (kTypeJpeg == type) ? LV_COLOR_FORMAT_RGB888 : LV_COLOR_FORMAT_ARGB8888;
    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.w = width;
    dsc->header.h = height;
    dsc->header.stride = width * ((kTypeJpeg == type) ? 3 : 4);
    dsc->data_size = width * height * ((kTypeJpeg == type) ? 3 : 4);
    dsc->data = image_arr;

    m_image_dsc = dsc;
    m_image_from_image_ins = false;

    // 已设置挖空区域，则不设置图像资源
    if (Rect() != m_clip_area) {
        ClipImage(m_clip_area);
    }
    // 设置图像资源
    else {
        lv_image_set_src(m_object, dsc);
    }
}

/**
 * @brief 拷贝顶部图像区域
 * @param image_dsc 图像数据信息指针
 * @param image_vec 挖空图像存储容器
 * @param clip_rect 挖空区域
 */
static void ClipTopArea(lv_image_dsc_t* image_dsc, Image::ImageVec& image_vec, const Rect& clip_rect) {
    if (!image_dsc || clip_rect.m_y <= 0) { return; }

    // 像素长度
    const int pixel_len = (LV_COLOR_FORMAT_RGB888 == image_dsc->header.cf) ? 3 : 4;
    // 图像宽
    const int width = image_dsc->header.w;
    // 图像高
    const int height = clip_rect.m_y;
    // 图像数据长度
    const int image_size = width * height * pixel_len;

    // 创建图像存储空间
    uint8_t* cache = new uint8_t[image_size];

    // 拷贝图像数据
    std::memcpy(cache, image_dsc->data, image_size);

    lv_image_dsc_t* dsc = new lv_image_dsc_t;
    std::memset(dsc, 0, sizeof(lv_image_dsc_t));
    dsc->header.cf = image_dsc->header.cf;
    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.w = width;
    dsc->header.h = height;
    dsc->header.stride = width * pixel_len;
    dsc->data_size = image_size;
    dsc->data = cache;

    // 保存数据
    Image::ImageClip image_clip;
    image_clip.m_image_dsc = dsc;
    image_clip.m_Image_area = Rect(0, 0, width, height);

    image_vec.push_back(image_clip);
}

/**
 * @brief 拷贝底部图像区域
 * @param image_dsc 图像数据信息指针
 * @param image_vec 挖空图像存储容器
 * @param clip_rect 挖空区域
 */
static void ClipBottomArea(lv_image_dsc_t* image_dsc, Image::ImageVec& image_vec, const Rect& clip_rect) {
    const int image_height = image_dsc ? static_cast<int>(image_dsc->header.h) : 0;
    if (!image_dsc || clip_rect.m_y + clip_rect.m_height >= image_height) { return; }

    // 像素长度
    const int pixel_len = (LV_COLOR_FORMAT_RGB888 == image_dsc->header.cf) ? 3 : 4;
    // 图像宽
    const int width = image_dsc->header.w;
    // 图像高
    const int height = image_height - clip_rect.m_y - clip_rect.m_height;
    // 图像数据长度
    const int image_size = width * height * pixel_len;

    // 创建图像存储空间
    uint8_t* cache = new uint8_t[image_size];

    // 拷贝图像数据
    const int offset = width * (clip_rect.m_y + clip_rect.m_height) * pixel_len;
    std::memcpy(cache, image_dsc->data + offset, image_size);

    lv_image_dsc_t* dsc = new lv_image_dsc_t;
    std::memset(dsc, 0, sizeof(lv_image_dsc_t));
    dsc->header.cf = image_dsc->header.cf;
    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.w = width;
    dsc->header.h = height;
    dsc->header.stride = width * pixel_len;
    dsc->data_size = image_size;
    dsc->data = cache;

    // 保存数据
    Image::ImageClip image_clip;
    image_clip.m_image_dsc = dsc;
    image_clip.m_Image_area = Rect(0, clip_rect.m_y + clip_rect.m_height, width, height);

    image_vec.push_back(image_clip);
}

/**
 * @brief 拷贝左边图像区域
 * @param image_dsc 图像数据信息指针
 * @param image_vec 挖空图像存储容器
 * @param clip_rect 挖空区域
 */
static void ClipLeftArea(lv_image_dsc_t* image_dsc, Image::ImageVec& image_vec, const Rect& clip_rect) {
    if (!image_dsc || clip_rect.m_x <= 0) { return; }

    // 像素长度
    const int pixel_len = (LV_COLOR_FORMAT_RGB888 == image_dsc->header.cf) ? 3 : 4;
    const int image_height = static_cast<int>(image_dsc->header.h);
    // 图像宽
    const int width = clip_rect.m_x;
    // 图像高
    const int height = (clip_rect.m_y + clip_rect.m_height <= image_height) ? clip_rect.m_height : image_height - clip_rect.m_y;
    // 图像数据长度
    const int image_size = width * height * pixel_len;

    // 创建图像存储空间
    uint8_t* cache = new uint8_t[image_size];

    // 拷贝图像数据
    const int offset = image_dsc->header.w * clip_rect.m_y * pixel_len;
    for (int level1 = 0; level1 < height; ++level1) {
        std::memcpy(cache + level1 * width * pixel_len, image_dsc->data + offset + level1 * image_dsc->header.w * pixel_len, width * pixel_len);
    }

    lv_image_dsc_t* dsc = new lv_image_dsc_t;
    std::memset(dsc, 0, sizeof(lv_image_dsc_t));
    dsc->header.cf = image_dsc->header.cf;
    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.w = width;
    dsc->header.h = height;
    dsc->header.stride = width * pixel_len;
    dsc->data_size = image_size;
    dsc->data = cache;

    // 保存数据
    Image::ImageClip image_clip;
    image_clip.m_image_dsc = dsc;
    image_clip.m_Image_area = Rect(0, clip_rect.m_y, width, height);

    image_vec.push_back(image_clip);
}

/**
 * @brief 拷贝右边图像区域
 * @param image_dsc 图像数据信息指针
 * @param image_vec 挖空图像存储容器
 * @param clip_rect 挖空区域
 */
static void ClipRightArea(lv_image_dsc_t* image_dsc, Image::ImageVec& image_vec, const Rect& clip_rect) {
    const int image_width = image_dsc ? static_cast<int>(image_dsc->header.w) : 0;
    const int image_height = image_dsc ? static_cast<int>(image_dsc->header.h) : 0;
    if (!image_dsc || clip_rect.m_x + clip_rect.m_width >= image_width) { return; }

    // 像素长度
    const int pixel_len = (LV_COLOR_FORMAT_RGB888 == image_dsc->header.cf) ? 3 : 4;
    // 图像宽
    const int width = image_width - clip_rect.m_x - clip_rect.m_width;
    // 图像高
    const int height = (clip_rect.m_y + clip_rect.m_height <= image_height) ? clip_rect.m_height : image_height - clip_rect.m_y;
    // 图像数据长度
    const int image_size = width * height * pixel_len;

    // 创建图像存储空间
    uint8_t* cache = new uint8_t[image_size];

    // 拷贝图像数据
    const int offset = image_dsc->header.w * clip_rect.m_y * pixel_len;
    for (int level1 = 0; level1 < height; ++level1) {
        std::memcpy(cache + level1 * width * pixel_len, image_dsc->data + offset     // 跳过顶部
            + level1 * image_dsc->header.w * pixel_len                              // 跳过拷贝的行
            + (clip_rect.m_x + clip_rect.m_width) * pixel_len,                      // 跳过左边区域
            width * pixel_len);
    }

    lv_image_dsc_t* dsc = new lv_image_dsc_t;
    std::memset(dsc, 0, sizeof(lv_image_dsc_t));
    dsc->header.cf = image_dsc->header.cf;
    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.w = width;
    dsc->header.h = height;
    dsc->header.stride = width * pixel_len;
    dsc->data_size = image_size;
    dsc->data = cache;

    // 保存数据
    Image::ImageClip image_clip;
    image_clip.m_image_dsc = dsc;
    image_clip.m_Image_area = Rect(clip_rect.m_x + clip_rect.m_width, clip_rect.m_y, width, height);

    image_vec.push_back(image_clip);
}

/**
 * @brief 自定义绘制回调函数
 * @param e 回调事件
 */
static void CustomDrawMain(lv_event_t* e) {
    // 获取传参数据
    Image* object = (Image*)lv_event_get_user_data(e);

    // 获取绘制上下文
    lv_layer_t* layer = lv_event_get_layer(e);

    // 定义图片绘制区域
    lv_area_t img_coords;

    // 初始化图片绘制描述符
    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);

    Image::ImageVec clip_image = object->GetClipImage();
    for (auto& iter : clip_image) {
        lv_area_set(&img_coords, iter.m_Image_area.m_x, iter.m_Image_area.m_y,
            iter.m_Image_area.m_x + iter.m_Image_area.m_width - 1, iter.m_Image_area.m_y + iter.m_Image_area.m_height - 1);
        img_dsc.src = iter.m_image_dsc;

        lv_draw_image(layer, &img_dsc, &img_coords);
    }
}

void Image::SetClipArea(const Rect& trans_rect) {
    m_clip_area = trans_rect;
}

void Image::ResetClipArea() {
    // 注销自定义绘制事件
    lv_obj_remove_event_cb(m_object, CustomDrawMain);
    // 释放先前切分数据
    RealeseClipImage();
    // 清空挖空区域
    m_clip_area = Rect();
}

Image::ImageVec Image::GetClipImage() {
    return m_clip_image;
}

void Image::ClipImage(const Rect& trans_rect) {
    // 释放先前切分数据
    RealeseClipImage();
    // 拆分顶部图片
    ClipTopArea(static_cast<lv_image_dsc_t*>(m_image_dsc), m_clip_image, trans_rect);
    // 拆分底部数据
    ClipBottomArea(static_cast<lv_image_dsc_t*>(m_image_dsc), m_clip_image, trans_rect);
    // 拆分左边数据
    ClipLeftArea(static_cast<lv_image_dsc_t*>(m_image_dsc), m_clip_image, trans_rect);
    // 拆分右边数据
    ClipRightArea(static_cast<lv_image_dsc_t*>(m_image_dsc), m_clip_image, trans_rect);

    // 释放加载的图片数据
    ReleaseImageDsc();
    // 注册自定义绘制事件
    lv_obj_add_event_cb(m_object, CustomDrawMain, LV_EVENT_DRAW_MAIN, this);
}

void Image::RealeseClipImage() {
    for (auto iter : m_clip_image) {
        lv_image_dsc_t* dsc = static_cast<lv_image_dsc_t*>(iter.m_image_dsc);
        if (!dsc) {
            continue;
        }
        delete[] dsc->data;
        delete dsc;
    }
    m_clip_image.clear();
}
