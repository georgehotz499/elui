#include "dashboard.h"

extern "C" {
#include "lvgl/lvgl.h"
}

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>


namespace {
    constexpr double kPi = 3.14159265358979323846;

    struct IndicatorMask {
        lv_image_dsc_t* m_image{ nullptr };
        int m_width{ 0 };
        int m_height{ 0 };
        int m_min{ 0 };
        int m_max{ 0 };
        int m_value{ 0 };
        int m_start_angle{ 0 };
        int m_range_angle{ 0 };
    };

    std::map<Dashboard*, IndicatorMask> g_indicator_mask;
}

/**
 * @brief 计算当前值在最大最小值之间的比例
 * @param dashboard 控件指针
 */
static double CalculateValueRatio(Dashboard* dashboard) {
    const int min_value = dashboard->GetMin();
    const int max_value = dashboard->GetMax();

    if (max_value <= min_value) {
        return 0.0;
    }

    return 1.0 * (dashboard->GetValue() - min_value) / (max_value - min_value);
}

/**
 * @brief 转换不透明度格式
 * @param opacity 控件不透明度
 */
static uint8_t ToLvOpacity(int opacity) {
    return static_cast<uint8_t>(opacity * 255 / 100);
}

/**
 * @brief 归一化角度到0~360
 * @param angle 角度
 */
static double NormalizeAngle(double angle) {
    while (360.0 <= angle) {
        angle -= 360.0;
    }
    while (angle < 0.0) {
        angle += 360.0;
    }

    return angle;
}

/**
 * @brief 销毁前景进度遮罩
 * @param dashboard 控件指针
 */
static void DestroyIndicatorMask(Dashboard* dashboard) {
    auto iter = g_indicator_mask.find(dashboard);
    if (iter == g_indicator_mask.end()) {
        return;
    }

    lv_image_dsc_t* image = iter->second.m_image;
    if (image) {
        std::free((void*)image->data);
        delete image;
    }

    g_indicator_mask.erase(iter);
}

/**
 * @brief 创建前景进度遮罩
 * @param width 遮罩宽度
 * @param height 遮罩高度
 */
static lv_image_dsc_t* CreateIndicatorMask(int width, int height) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    const size_t data_size = static_cast<size_t>(width) * static_cast<size_t>(height);
    uint8_t* data = static_cast<uint8_t*>(std::malloc(data_size));
    if (!data) {
        return nullptr;
    }

    lv_image_dsc_t* image = new lv_image_dsc_t;
    std::memset(image, 0, sizeof(lv_image_dsc_t));
    image->header.cf = LV_COLOR_FORMAT_A8;
    image->header.magic = LV_IMAGE_HEADER_MAGIC;
    image->header.w = width;
    image->header.h = height;
    image->header.stride = width;
    image->data_size = static_cast<uint32_t>(data_size);
    image->data = data;

    return image;
}

/**
 * @brief 判断角度是否在当前进度范围内
 * @param angle 当前角度
 * @param start_angle 起始角度
 * @param progress_angle 进度角度
 */
static bool IsAngleInProgress(double angle, double start_angle, double progress_angle) {
    return NormalizeAngle(angle - start_angle) <= progress_angle;
}

/**
 * @brief 刷新前景进度遮罩数据
 * @param dashboard 控件指针
 * @param mask 遮罩图片
 */
static void UpdateIndicatorMaskData(Dashboard* dashboard, lv_image_dsc_t* mask) {
    const int width = mask->header.w;
    const int height = mask->header.h;
    const size_t data_size = static_cast<size_t>(width) * static_cast<size_t>(height);
    uint8_t* data = const_cast<uint8_t*>(mask->data);

    const double progress_angle = CalculateValueRatio(dashboard) * dashboard->GetRangeAngle();
    if (progress_angle <= 0.0) {
        std::memset(data, 0, data_size);
        return;
    }

    if (360.0 <= progress_angle) {
        std::memset(data, 0xFF, data_size);
        return;
    }

    const double start_angle = NormalizeAngle(dashboard->GetStartAngle());
    const double center_x = (width - 1) / 2.0;
    const double center_y = (height - 1) / 2.0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const double dx = x - center_x;
            const double dy = y - center_y;
            const double angle = NormalizeAngle(std::atan2(dy, dx) * 180.0 / kPi);
            data[y * width + x] = IsAngleInProgress(angle, start_angle, progress_angle) ? 0xFF : 0;
        }
    }
}

/**
 * @brief 判断前景进度遮罩是否需要刷新
 * @param dashboard 控件指针
 * @param mask 遮罩缓存
 * @param image 前景图片
 */
static bool IsIndicatorMaskValid(Dashboard* dashboard, const IndicatorMask& mask, lv_image_dsc_t* image) {
    return mask.m_image
        && mask.m_width == static_cast<int>(image->header.w)
        && mask.m_height == static_cast<int>(image->header.h)
        && mask.m_min == dashboard->GetMin()
        && mask.m_max == dashboard->GetMax()
        && mask.m_value == dashboard->GetValue()
        && mask.m_start_angle == dashboard->GetStartAngle()
        && mask.m_range_angle == dashboard->GetRangeAngle();
}

/**
 * @brief 获取前景进度遮罩
 * @param dashboard 控件指针
 * @param image 前景图片
 */
static lv_image_dsc_t* GetIndicatorMask(Dashboard* dashboard, lv_image_dsc_t* image) {
    IndicatorMask& mask = g_indicator_mask[dashboard];

    const int width = image->header.w;
    const int height = image->header.h;
    if (mask.m_image && (mask.m_width != width || mask.m_height != height)) {
        DestroyIndicatorMask(dashboard);
        return GetIndicatorMask(dashboard, image);
    }

    if (!mask.m_image) {
        mask.m_image = CreateIndicatorMask(width, height);
        if (!mask.m_image) {
            return nullptr;
        }
    }

    if (!IsIndicatorMaskValid(dashboard, mask, image)) {
        UpdateIndicatorMaskData(dashboard, mask.m_image);
        mask.m_width = width;
        mask.m_height = height;
        mask.m_min = dashboard->GetMin();
        mask.m_max = dashboard->GetMax();
        mask.m_value = dashboard->GetValue();
        mask.m_start_angle = dashboard->GetStartAngle();
        mask.m_range_angle = dashboard->GetRangeAngle();
    }

    return mask.m_image;
}

/**
 * @brief 计算图片居中显示区域
 * @param rect 控件全局坐标
 * @param image 图片
 * @param image_area 图片显示区域
 */
static bool CalculateImageArea(const Rect& rect, lv_image_dsc_t* image, lv_area_t* image_area) {
    if (!image) {
        return false;
    }

    const int image_width = image->header.w;
    const int image_height = image->header.h;
    if (image_width <= 0 || image_height <= 0) {
        return false;
    }

    const int center_x = rect.m_x + rect.m_width / 2;
    const int center_y = rect.m_y + rect.m_height / 2;
    const int image_x = center_x - image_width / 2;
    const int image_y = center_y - image_height / 2;

    image_area->x1 = image_x;
    image_area->y1 = image_y;
    image_area->x2 = image_x + image_width - 1;
    image_area->y2 = image_y + image_height - 1;

    return true;
}

/**
 * @brief 绘制图片
 * @param layer 控件图层
 * @param image 图片
 * @param rect 控件全局坐标
 * @param opacity 不透明度
 * @param mask 遮罩图片
 */
static void DrawImage(lv_layer_t* layer, ImageDes image, const Rect& rect, int opacity, const lv_image_dsc_t* mask = nullptr) {
    lv_image_dsc_t* image_dsc = static_cast<lv_image_dsc_t*>(image);
    lv_area_t image_area;
    if (!CalculateImageArea(rect, image_dsc, &image_area)) {
        return;
    }

    lv_draw_image_dsc_t draw_image;
    lv_draw_image_dsc_init(&draw_image);
    draw_image.src = image_dsc;
    draw_image.opa = ToLvOpacity(opacity);
    draw_image.bitmap_mask_src = mask;

    lv_draw_image(layer, &draw_image, &image_area);
}

/**
 * @brief 绘制背景图片
 * @param layer 控件图层
 * @param dashboard 控件指针
 * @param rect 控件全局坐标
 */
static void DrawImageMain(lv_layer_t* layer, Dashboard* dashboard, const Rect& rect) {
    DrawImage(layer, dashboard->GetImageMain(), rect, dashboard->GetOpacity());
}

/**
 * @brief 绘制前景图片
 * @param layer 控件图层
 * @param dashboard 控件指针
 * @param rect 控件全局坐标
 */
static void DrawImageIndicator(lv_layer_t* layer, Dashboard* dashboard, const Rect& rect) {
    lv_image_dsc_t* image = static_cast<lv_image_dsc_t*>(dashboard->GetImageIndicator());
    if (!image || CalculateValueRatio(dashboard) <= 0.0) {
        return;
    }

    lv_image_dsc_t* mask = GetIndicatorMask(dashboard, image);
    if (!mask) {
        return;
    }

    DrawImage(layer, image, rect, dashboard->GetOpacity(), mask);
}

/**
 * @brief 计算指针当前角度
 * @param dashboard 控件指针
 */
static double CalculatePointerAngle(Dashboard* dashboard) {
    return NormalizeAngle(
        dashboard->GetStartAngle() + CalculateValueRatio(dashboard) * dashboard->GetRangeAngle());
}

/**
 * @brief 绘制图片指针
 * @param layer 控件图层
 * @param dashboard 控件指针
 * @param rect 控件全局坐标
 */
static void DrawImagePointer(lv_layer_t* layer, Dashboard* dashboard, const Rect& rect) {
    lv_image_dsc_t* image_dsc = static_cast<lv_image_dsc_t*>(dashboard->GetImagePointer());
    lv_area_t image_area;
    if (!CalculateImageArea(rect, image_dsc, &image_area)) {
        return;
    }

    const int image_width = image_dsc->header.w;
    const int image_height = image_dsc->header.h;

    const double pointer_angle = CalculatePointerAngle(dashboard);
    const double rotation_angle = NormalizeAngle(pointer_angle - dashboard->GetImagePointerBaseAngle());

    lv_draw_image_dsc_t draw_image;
    lv_draw_image_dsc_init(&draw_image);
    draw_image.src = image_dsc;
    draw_image.opa = ToLvOpacity(dashboard->GetOpacity());
    draw_image.rotation = static_cast<int32_t>(rotation_angle * 10);
    draw_image.pivot.x = image_width / 2;
    draw_image.pivot.y = image_height / 2;

    lv_draw_image(layer, &draw_image, &image_area);
}

/**
 * @brief 事件处理
 * @param event 事件信息
 */
static void EventHandler(lv_event_t* event) {
    lv_event_code_t code = lv_event_get_code(event);
    Dashboard* dashboard = static_cast<Dashboard*>(lv_event_get_user_data(event));
    lv_layer_t* layer = lv_event_get_layer(event);

    switch (code) {
    case LV_EVENT_DRAW_MAIN:
    {
        const Rect rect = dashboard->GetGlobalGeometry();
        DrawImageMain(layer, dashboard, rect);
        DrawImageIndicator(layer, dashboard, rect);
        DrawImagePointer(layer, dashboard, rect);
    }
    break;
    default:
        break;
    }
}

static lv_event_code_t event_list[] = {
    LV_EVENT_DRAW_MAIN,
};

Dashboard::Dashboard(std::string name, Widget* parent) : Widget(name, parent) {
    Create(parent->GetLayer());

    for (int i = 0; i < sizeof(event_list) / sizeof(event_list[0]); i++) {
        lv_obj_add_event_cb(m_object, EventHandler, event_list[i], this);
    }
}

Dashboard::~Dashboard() {
    DestroyIndicatorMask(this);
    ImageIns::DestroyImageDes(m_image_main);
    ImageIns::DestroyImageDes(m_image_indicator);
    ImageIns::DestroyImageDes(m_image_pointer);
}

void Dashboard::Create(ScreenLayer layer) {
    m_object = lv_obj_create(layer);

    lv_obj_set_style_radius(m_object, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m_object, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(m_object, 0, LV_PART_MAIN);
    lv_obj_remove_flag(m_object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(m_object, LV_OPA_TRANSP, 0);
}

void Dashboard::SetImageMain(const std::string& path) {
    ImageIns::DestroyImageDes(m_image_main);
    m_image_main = ImageIns::ParseImage(path);
    Refresh();
}

ImageDes Dashboard::GetImageMain() {
    return m_image_main;
}

void Dashboard::SetImageIndicator(const std::string& path) {
    ImageIns::DestroyImageDes(m_image_indicator);
    DestroyIndicatorMask(this);
    m_image_indicator = ImageIns::ParseImage(path);
    Refresh();
}

ImageDes Dashboard::GetImageIndicator() {
    return m_image_indicator;
}

void Dashboard::SetOpacity(int opacity) {
    if (opacity < 0 || 100 < opacity) {
        return;
    }
    m_opacity = opacity;
    Refresh();
}

int Dashboard::GetOpacity() {
    return m_opacity;
}

void Dashboard::SetStartAngle(int angle) {
    m_start_angle = angle;
    Refresh();
}

int Dashboard::GetStartAngle() {
    return m_start_angle;
}

void Dashboard::SetRangeAngle(int angle) {
    if (angle <= 0) {
        return;
    }
    m_range_angle = angle;
    Refresh();
}

int Dashboard::GetRangeAngle() {
    return m_range_angle;
}

void Dashboard::SetMin(int value) {
    if (m_max <= value) {
        return;
    }
    m_min = value;
    ClampValue();
    Refresh();
}

int Dashboard::GetMin() {
    return m_min;
}

void Dashboard::SetMax(int value) {
    if (value <= m_min) {
        return;
    }
    m_max = value;
    ClampValue();
    Refresh();
}

int Dashboard::GetMax() {
    return m_max;
}

void Dashboard::SetValue(int value) {
    m_value = value;
    ClampValue();
    Refresh();
}

int Dashboard::GetValue() {
    return m_value;
}

void Dashboard::SetImagePointer(const std::string& path) {
    ImageIns::DestroyImageDes(m_image_pointer);
    m_image_pointer = ImageIns::ParseImage(path);
    Refresh();
}

ImageDes Dashboard::GetImagePointer() {
    return m_image_pointer;
}

void Dashboard::SetImagePointerBaseAngle(int angle) {
    m_image_pointer_base_angle = angle;
    Refresh();
}

int Dashboard::GetImagePointerBaseAngle() {
    return m_image_pointer_base_angle;
}

void Dashboard::ClampValue() {
    if (m_value < m_min) {
        m_value = m_min;
    }
    else if (m_max < m_value) {
        m_value = m_max;
    }
}
