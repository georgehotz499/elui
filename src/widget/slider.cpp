#include "slider.h"

#include "core/log.h"

extern "C" {
#include "lvgl/lvgl.h"
}


/**
 * @brief 绘制滑条背景
 * @param layer 控件图层
 * @param slider 控件指针
 * @param rect 控件全局坐标
 */
static void DrawMain(lv_layer_t* layer, Slider* slider, const Rect& rect) {
    // 初始化矩形绘制描述符
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);

    // 绑定图片资源，自动拉伸填充矩形
    rect_dsc.bg_image_src = slider->GetImageMain();
    rect_dsc.bg_image_opa = LV_OPA_100;
    rect_dsc.bg_image_tiled = 0; // 拉伸铺满，不平铺

    const Color color = slider->GetMainColor();
    rect_dsc.bg_color.red = color.m_r;
    rect_dsc.bg_color.green = color.m_g;
    rect_dsc.bg_color.blue = color.m_b;
    rect_dsc.bg_opa = static_cast<uint8_t>(slider->GetOpacity() * 255 / 100);
    rect_dsc.radius = slider->GetRadius();

    // 定义矩形区域
    lv_area_t area = { rect.m_x, rect.m_y, rect.m_x + rect.m_width - 1, rect.m_y + rect.m_height - 1 };

    // 横向
    if (Slider::kHor == slider->GetOrientation()) {
        // 计算宽度偏移
        const int width_offset = (rect.m_height + 1) / 2;
        // 计算宽度
        area.x1 += width_offset;
        area.x2 -= width_offset;
        // 计算高度
        if (slider->GetPadding() < area.y2 - area.y1) {
            area.y1 += slider->GetPadding();
            area.y2 -= slider->GetPadding();
        }
    }
    // 纵向
    else {
        // 计算高度偏移
        const int height_offset = (rect.m_width + 1) / 2;
        // 计算高度
        area.y1 += height_offset;
        area.y2 -= height_offset;
        // 计算宽度
        if (slider->GetPadding() < area.x2 - area.x1) {
            area.x1 += slider->GetPadding();
            area.x2 -= slider->GetPadding();
        }
    }

    // 绘制矩形
    lv_draw_rect(layer, &rect_dsc, &area);
}

/**
 * @brief 绘制滑条进度指示器颜色
 * @param layer 控件图层
 * @param slider 控件指针
 * @param rect 控件全局坐标
 */
static void DrawIndicatorColor(lv_layer_t* layer, Slider* slider, const Rect& rect) {
    // 初始化矩形绘制描述符
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);

    // 绑定图片资源，自动拉伸填充矩形
    rect_dsc.bg_image_src = slider->GetImageIndicator();
    rect_dsc.bg_image_opa = LV_OPA_100;
    rect_dsc.bg_image_tiled = 0; // 拉伸铺满，不平铺

    const Color color = slider->GetIndColor();
    rect_dsc.bg_color.red = color.m_r;
    rect_dsc.bg_color.green = color.m_g;
    rect_dsc.bg_color.blue = color.m_b;
    rect_dsc.bg_opa = static_cast<uint8_t>(slider->GetOpacity() * 255 / 100);
    rect_dsc.radius = slider->GetRadius();

    // 定义矩形区域
    lv_area_t area = { rect.m_x, rect.m_y, rect.m_x + rect.m_width - 1, rect.m_y + rect.m_height - 1 };

    // 横向
    if (Slider::kHor == slider->GetOrientation()) {
        // 计算宽度偏移
        const int width_offset = (rect.m_height + 1) / 2;
        // 计算宽度
        area.x1 += width_offset;
        area.x2 = area.x1 + 1.0 * (slider->GetProgress() - slider->GetMinValue()) / (slider->GetMaxValue() - slider->GetMinValue()) * (rect.m_width - 2 * width_offset);
        // 计算高度
        if (slider->GetPadding() < area.y2 - area.y1) {
            area.y1 += slider->GetPadding();
            area.y2 -= slider->GetPadding();
        }
    }
    // 纵向
    else {
        // 计算高度偏移
        const int height_offset = (rect.m_width + 1) / 2;
        // 计算高度
        area.y2 -= height_offset;
        area.y1 = area.y2 - 1.0 * (slider->GetProgress() - slider->GetMinValue()) / (slider->GetMaxValue() - slider->GetMinValue()) * (rect.m_height - 2 * height_offset);
        // 计算宽度
        if (slider->GetPadding() < area.x2 - area.x1) {
            area.x1 += slider->GetPadding();
            area.x2 -= slider->GetPadding();
        }
    }

    // 绘制矩形
    lv_draw_rect(layer, &rect_dsc, &area);
}

/**
 * @brief 绘制滑条knob颜色
 * @param layer 控件图层
 * @param slider 控件指针
 * @param rect 控件全局坐标
 */
static void DrawKnob(lv_layer_t* layer, Slider* slider, const Rect& rect) {
    // 定义矩形区域
    lv_area_t area = { rect.m_x, rect.m_y, rect.m_x + rect.m_width - 1, rect.m_y + rect.m_height - 1 };

    // 计算宽度偏移
    const int width_offset = (rect.m_height + 1) / 2;
    // 计算高度偏移
    const int height_offset = (rect.m_width + 1) / 2;
    // 圆心
    int center_x{ rect.m_x };
    int center_y{ rect.m_y };
    // 半径
    int radius{ width_offset };

    // 横向
    if (Slider::kHor == slider->GetOrientation()) {
        center_x = rect.m_x + width_offset + 1.0 * (slider->GetProgress() - slider->GetMinValue()) / (slider->GetMaxValue() - slider->GetMinValue()) * (rect.m_width - 2 * width_offset);
        center_y = rect.m_y + width_offset;
        radius = width_offset;
    }
    // 纵向
    else {
        center_x = rect.m_x + height_offset;
        center_y = 1 + area.y2 - height_offset - 1.0 * (slider->GetProgress() - slider->GetMinValue()) / (slider->GetMaxValue() - slider->GetMinValue()) * (rect.m_height - 2 * height_offset);
        radius = height_offset;
    }

    // 定义并初始化dsc
    lv_draw_arc_dsc_t arc_dsc;
    lv_draw_arc_dsc_init(&arc_dsc);

    // 配置参数
    Color knob_color = slider->GetKnobColor();
    arc_dsc.color = lv_color_make(knob_color.m_r, knob_color.m_g, knob_color.m_b);
    arc_dsc.opa = static_cast<uint8_t>(slider->GetOpacity() * 255 / 100);
    arc_dsc.rounded = 0; // 1:圆弧两端圆角，0:直角平头
    // 绑定图片资源
    arc_dsc.img_src = slider->GetImageKnob();

    arc_dsc.center.x = center_x;    // 圆心X
    arc_dsc.center.y = center_y;    // 圆心Y
    arc_dsc.radius = radius;        // 外半径
    arc_dsc.width = radius;         // 厚度等于半径 → 实心填充
    arc_dsc.start_angle = 0;        // 起始0°
    arc_dsc.end_angle = 360;       // 结束360°，完整圆环

    // 执行绘制
    lv_draw_arc(layer, &arc_dsc);
}

/**
 * @brief 绘制进度条图片
 * @param layer 控件图层
 * @param slider 控件指针
 * @param rect 控件全局坐标
 */
static void DrawIndicatorImage(lv_layer_t* layer, Slider* slider, const Rect& rect) {
    lv_draw_image_dsc_t draw_image;
    lv_draw_image_dsc_init(&draw_image);
    draw_image.src = slider->GetImageIndicator();
    draw_image.opa = static_cast<uint8_t>(slider->GetOpacity() * 255 / 100);

    // 定义矩形区域
    lv_area_t area = { rect.m_x, rect.m_y, rect.m_x + rect.m_width - 1, rect.m_y + rect.m_height - 1 };

    // 横向
    if (Slider::kHor == slider->GetOrientation()) {
        // 计算宽度偏移
        const int width_offset = (rect.m_height + 1) / 2;
        // 计算宽度
        area.x1 += width_offset;
        area.x2 = -1 + area.x1 + 1.0 * (slider->GetProgress() - slider->GetMinValue()) / (slider->GetMaxValue() - slider->GetMinValue()) * (rect.m_width - 2 * width_offset);
        // 计算高度
        if (slider->GetPadding() < area.y2 - area.y1) {
            area.y1 += slider->GetPadding();
            area.y2 -= slider->GetPadding();
        }
    }
    // 纵向
    else {
        // 计算高度偏移
        const int height_offset = (rect.m_width + 1) / 2;
        // 计算高度
        area.y2 -= height_offset;
        area.y1 = 1 + area.y2 - 1.0 * (slider->GetProgress() - slider->GetMinValue()) / (slider->GetMaxValue() - slider->GetMinValue()) * (rect.m_height - 2 * height_offset);
        // 计算宽度
        if (slider->GetPadding() < area.x2 - area.x1) {
            area.x1 += slider->GetPadding();
            area.x2 -= slider->GetPadding();
        }
        if (draw_image.src && area.y1 < area.y2) {
            lv_image_dsc_t* origin_des = (lv_image_dsc_t*)draw_image.src;

            lv_image_dsc_t* image_des = (lv_image_dsc_t*)slider->GetImageIndicatorReal();
            memset(image_des, 0, sizeof(lv_image_dsc_t));
            image_des->header.cf = origin_des->header.cf;
            image_des->header.magic = LV_IMAGE_HEADER_MAGIC;
            image_des->header.w = origin_des->header.w;
            image_des->header.h = area.y2 - area.y1 + 1;
            image_des->header.stride = origin_des->header.stride;
            image_des->data_size = origin_des->header.stride * image_des->header.h;
            image_des->data = origin_des->data + origin_des->header.stride * (slider->GetGeometry().m_height - image_des->header.h);
            draw_image.src = image_des;
        }
    }

    // 绘制图片
    lv_draw_image(layer, &draw_image, &area);
}

/**
 * @brief 事件处理
 * @param event 事件信息
 */
static void EventHandler(lv_event_t* event) {
    // 获取事件码
    lv_event_code_t code = lv_event_get_code(event);
    Slider* slider = static_cast<Slider*>(lv_event_get_user_data(event));
    // 获取图层指针
    lv_layer_t* layer = lv_event_get_layer(event);

    switch (code) {
    case LV_EVENT_DRAW_MAIN:
    {
        // 控件全局坐标
        const Rect rect = slider->GetGlobalGeometry();

        // 绘制背景
        DrawMain(layer, slider, rect);
        // 绘制进度条
        if (slider->GetImageIndicator()) {
            DrawIndicatorImage(layer, slider, rect);
        }
        else {
            DrawIndicatorColor(layer, slider, rect);
        }
        // 绘制knob
        if (slider->IsKnobVisible()) {
            DrawKnob(layer, slider, rect);
        }
    }
    break;
    default:
        break;
    }
}

// 定义要监听的事件列表
static lv_event_code_t event_list[] = {
    // 绘制主体
    LV_EVENT_DRAW_MAIN, // 26
};

Slider::Slider(std::string name, Widget* parent) : Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());

    // 绑定点击事件
    for (int i = 0; i < sizeof(event_list) / sizeof(event_list[0]); i++) {
        lv_obj_add_event_cb(m_object, EventHandler, event_list[i], this);
    }

    // 注册触摸回调
    AddTouchListener(this);
}

Slider::~Slider() {
    // 析构背景图片
    ImageIns::DestroyImageDes(m_image_main);
    // 析构进度图片
    ImageIns::DestroyImageDes(m_image_indicator);
    // 析构进度指示图实际进度的存储对象
    delete (lv_image_dsc_t*)m_image_indicator_real;
    // 析构knob图片
    ImageIns::DestroyImageDes(m_image_knob);
}

void Slider::Create(ScreenLayer layer) {
    m_object = lv_obj_create(layer);

    // 移除 圆角
    lv_obj_set_style_radius(m_object, 0, LV_PART_MAIN);

    // 移除 所有内边距（上、下、左、右）
    lv_obj_set_style_pad_all(m_object, 0, LV_PART_MAIN);

    // 关闭边框宽度 = 无边框
    lv_obj_set_style_border_width(m_object, 0, LV_PART_MAIN);

    // 禁止滚动
    lv_obj_remove_flag(m_object, LV_OBJ_FLAG_SCROLLABLE);

    // 设置背景完全透明
    lv_obj_set_style_bg_opa(m_object, LV_OPA_TRANSP, 0);
}

void Slider::SetMainColor(const Color& color) {
    m_color_main = color;
}

Color Slider::GetMainColor() {
    return m_color_main;
}

void Slider::SetIndColor(const Color& color) {
    m_color_indicator = color;
}

Color Slider::GetIndColor() {
    return m_color_indicator;
}

void Slider::SetKnobColor(const Color& color) {
    m_color_knob = color;
}

Color Slider::GetKnobColor() {
    return m_color_knob;
}

void Slider::SetOrientation(Orientation orientation) {
    m_orientation = orientation;
}

Slider::Orientation Slider::GetOrientation() {
    return m_orientation;
}

void Slider::SetRadius(int radius) {
    if (0 > radius) {
        return;
    }
    m_radius = radius;
}

int Slider::GetRadius() {
    return m_radius;
}

void Slider::SetOpacity(int opacity) {
    if (0 > opacity || 100 < opacity) {
        return;
    }
    m_opacity = opacity;
}

int Slider::GetOpacity() {
    return m_opacity;
}

void Slider::SetMinValue(int min) {
    if (m_max <= min) {
        return;
    }
    m_min = min;
}

int Slider::GetMinValue() {
    return m_min;
}

void Slider::SetMaxValue(int max) {
    if (max <= m_min) {
        return;
    }
    m_max = max;
}

int Slider::GetMaxValue() {
    return m_max;
}

void Slider::SetRange(int min, int max) {
    if (max <= min) {
        return;
    }
    m_min = min;
    m_max = max;
}

void Slider::SetProgress(int progress) {
    if (progress < m_min) {
        progress = m_min;
    }
    else if (m_max < progress) {
        progress = m_max;
    }
    m_progress = progress;

    // 刷新UI
    Refresh();
}

int Slider::GetProgress() {
    return m_progress;
}

void Slider::SetPadding(int paddding) {
    if (m_padding < 0) {
        return;
    }
    m_padding = paddding;
}

int Slider::GetPadding() {
    return m_padding;
}

void Slider::SetGeometry(int x, int y, int w, int h) {
    m_geometry.m_x = x;
    m_geometry.m_y = y;
    m_geometry.m_width = w;
    m_geometry.m_height = h;

    // 获取滑动方向
    const Slider::Orientation ori = GetOrientation();
    // 横向
    if (Slider::kHor == ori) {
        // 计算宽度偏移
        const int width_offset = (m_geometry.m_height + 1) / 2;
        x = x - width_offset;
        w = w + 2 * width_offset;

        Widget::SetGeometry(x, y, w, h);
    }
    // 纵向
    else {
        // 计算高度偏移
        const int height_offset = (m_geometry.m_width + 1) / 2;
        y = y - height_offset;
        h = h + 2 * height_offset;

        Widget::SetGeometry(x, y, w, h);
    }
}

Rect Slider::GetGeometry() {
    return m_geometry;
}

void Slider::SetImageMain(const std::string& path) {
    m_image_main = ImageIns::ParseImage(path);
}

ImageDes Slider::GetImageMain() {
    return m_image_main;
}

void Slider::SetImageIndicator(const std::string& path) {
    m_image_indicator = ImageIns::ParseImage(path);
}

ImageDes Slider::GetImageIndicator() {
    return m_image_indicator;
}

ImageDes Slider::GetImageIndicatorReal() {
    if (!m_image_indicator_real) {
        m_image_indicator_real = new lv_image_dsc_t;
        memset(m_image_indicator_real, 0, sizeof(lv_image_dsc_t));
    }
    return m_image_indicator_real;
}

void Slider::SetImageKnob(const std::string& path) {
    m_image_knob = ImageIns::ParseImage(path);
}

ImageDes Slider::GetImageKnob() {
    return m_image_knob;
}

void Slider::SetKnobVisible(bool is_visible) {
    m_knob_visible = is_visible;
}

bool Slider::IsKnobVisible() {
    return m_knob_visible;
}

void Slider::AddProgressCallback(ProgressCallback callback) {
    m_progress_callback = callback;
}

void Slider::ExecuteProgressCallback(Slider* slider, int progress) {
    if (m_progress_callback) {
        m_progress_callback(slider, progress);
    }
}

/**
* @brief 计算当前进度值
* @param slider 控件指针
* @param gesture 手势参数
* @return 返回进度值
*/
static int CalculateProgress(Slider* slider, const Gesture& gesture) {
    int progress{ 0 };

    // 获取滑动方向
    const Slider::Orientation ori = slider->GetOrientation();
    // 获取控件几何参数
    const Rect rect = slider->GetGeometry();
    // 横向
    if (Slider::kHor == ori) {
        // 计算宽度偏移
        const int width_offset = (rect.m_height + 1) / 2;
        int x_pos = gesture.m_point.m_x;
        x_pos = x_pos < width_offset ? 0 : (rect.m_width + width_offset < x_pos) ? rect.m_width : x_pos - width_offset;

        progress = 1.0 * x_pos / rect.m_width * (slider->GetMaxValue() - slider->GetMinValue());
    }
    // 纵向
    else {
        // 计算高度偏移
        const int height_offset = (rect.m_width + 1) / 2;
        int y_pos = gesture.m_point.m_y;
        y_pos = y_pos < height_offset ? 0 : (rect.m_height + height_offset < y_pos) ? rect.m_height : y_pos - height_offset;

        progress = 100 - 1.0 * y_pos / rect.m_height * (slider->GetMaxValue() - slider->GetMinValue());
    }

    return progress;
}

bool Slider::Touch(Widget* widget, const Gesture& gesture) {
    // 滑动(滑动动作很容易高频次触发，因此放在最前使得第一次就能命中)
    if (m_press_ctrl && Gesture::kMoving == gesture.m_event) {
        SetProgress(CalculateProgress(this, gesture));
        // 执行滑条滑动回调
        ExecuteProgressCallback(this, m_progress);
    }
    // 按下
    else if (Gesture::kPress == gesture.m_event) {
        m_press_ctrl = true;
        SetProgress(CalculateProgress(this, gesture));
        // 执行滑条滑动回调
        ExecuteProgressCallback(this, m_progress);
    }
    // 抬起
    else if (m_press_ctrl && Gesture::kRelease == gesture.m_event) {
        m_press_ctrl = false;
        SetProgress(CalculateProgress(this, gesture));
        // 执行滑条滑动回调
        ExecuteProgressCallback(this, m_progress);
    }

    return false;
}
