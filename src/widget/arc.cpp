#include "arc.h"

#include "core/log.h"

extern "C" {
#include "lvgl/lvgl.h"
}

#include <cmath>


/**
 * @brief 绘制滑条背景
 * @param layer 控件图层
 * @param slider 控件指针
 * @param rect 控件全局坐标
 */
static void DrawMain(lv_layer_t* layer, class Arc* arc, const Rect& rect) {
    // 半径
    const int radius = (rect.m_width + 1) / 2;

    // 中心点
    const int center_x = rect.m_x + radius;
    const int center_y = rect.m_y + radius;

    // 定义并初始化dsc
    lv_draw_arc_dsc_t arc_dsc;
    lv_draw_arc_dsc_init(&arc_dsc);

    // 配置参数
    Color main_color = arc->GetMainColor();
    arc_dsc.color = lv_color_make(main_color.m_r, main_color.m_g, main_color.m_b);
    arc_dsc.opa = static_cast<uint8_t>(arc->GetOpacity() * 255 / 100);
    arc_dsc.rounded = 1; // 1:圆弧两端圆角，0:直角平头
    // 绑定图片资源
    arc_dsc.img_src = arc->GetImageMain();

    arc_dsc.center.x = center_x;    // 圆心X
    arc_dsc.center.y = center_y;    // 圆心Y
    // 外半径
    arc_dsc.radius = radius - arc->GetPadding();
    // 厚度等于半径 → 实心填充
    arc_dsc.width = arc_dsc.radius - arc->GetPadding() - arc->GetUntouchableRadius();
    arc_dsc.start_angle = arc->GetStartAngle(); // 起始角度
    arc_dsc.end_angle = arc->GetStartAngle() + arc->GetRangeAngle(); // 终止角度

    // 执行绘制
    lv_draw_arc(layer, &arc_dsc);
}

/**
 * @brief 绘制滑条进度指示器
 * @param layer 控件图层
 * @param slider 控件指针
 * @param rect 控件全局坐标
 */
static void DrawIndicator(lv_layer_t* layer, class Arc* arc, const Rect& rect) {
    // 半径
    const int radius = (rect.m_width + 1) / 2;

    // 中心点
    const int center_x = rect.m_x + radius;
    const int center_y = rect.m_y + radius;

    // 定义并初始化dsc
    lv_draw_arc_dsc_t arc_dsc;
    lv_draw_arc_dsc_init(&arc_dsc);

    // 配置参数
    Color indicator_color = arc->GetIndColor();
    arc_dsc.color = lv_color_make(indicator_color.m_r, indicator_color.m_g, indicator_color.m_b);
    arc_dsc.opa = static_cast<uint8_t>(arc->GetOpacity() * 255 / 100);
    arc_dsc.rounded = 1; // 1:圆弧两端圆角，0:直角平头
    // 绑定图片资源
    arc_dsc.img_src = arc->GetImageIndicator();

    arc_dsc.center.x = center_x;    // 圆心X
    arc_dsc.center.y = center_y;    // 圆心Y
    // 外半径
    arc_dsc.radius = radius - arc->GetPadding();
    // 厚度等于半径 → 实心填充
    arc_dsc.width = arc_dsc.radius - arc->GetPadding() - arc->GetUntouchableRadius();
    // 起始角度
    arc_dsc.start_angle = arc->GetStartAngle();
    // 终止角度
    arc_dsc.end_angle = arc->GetStartAngle() + 1.0 * (arc->GetProgress() - arc->GetMin()) / (arc->GetMax() - arc->GetMin()) * arc->GetRangeAngle();

    // 执行绘制
    lv_draw_arc(layer, &arc_dsc);
}

/**
 * @brief 计算knob中心坐标
 * 0°右，90°下，180°左，270°上
 * @param cx 圆心X
 * @param cy 圆心Y
 * @param r 半径
 * @param deg 角度(0~360)
 * @param x 出参x坐标
 * @param y 出参y坐标
 */
static void CalculateKnobCenter(double cx, double cy, double r, double deg, int& x, int&y) {
    // 通过角度计算对应的弧度
    double rad = deg * 3.1415926 / 180.0;
    // 计算对应的x、y坐标
    x = cx + r * cos(rad);
    y = cy + r * sin(rad);
}

/**
 * @brief 绘制滑条knob
 * @param layer 控件图层
 * @param slider 控件指针
 * @param rect 控件全局坐标
 */
static void DrawKnob(lv_layer_t* layer, class Arc* arc, const Rect& rect) {
    // 半径
    const int radius = (rect.m_width + 1) / 2;

    // 中心点
    const int center_x = rect.m_x + radius;
    const int center_y = rect.m_y + radius;

    // 计算当前进度对应角度
    double angle = 1.0 * (arc->GetProgress() - arc->GetMin()) / (arc->GetMax() - arc->GetMin()) * arc->GetRangeAngle() + arc->GetStartAngle();
    // 角度控制在360°以内
    while (360 <= angle) {
        angle -= 360;
    }

    // 出参knob中心坐标
    int cx{ 0 };
    int cy{ 0 };
    const int r = radius - (radius - arc->GetUntouchableRadius()) / 2;
    CalculateKnobCenter(center_x, center_y, r, angle, cx, cy);
    // 定义并初始化dsc
    lv_draw_arc_dsc_t arc_dsc;
    lv_draw_arc_dsc_init(&arc_dsc);

    // 配置参数
    Color indicator_color = arc->GetKnobColor();
    arc_dsc.color = lv_color_make(indicator_color.m_r, indicator_color.m_g, indicator_color.m_b);
    arc_dsc.opa = static_cast<uint8_t>(arc->GetOpacity() * 255 / 100);
    arc_dsc.rounded = 0; // 1:圆弧两端圆角，0:直角平头
    // 绑定图片资源
    arc_dsc.img_src = arc->GetImageKnob();

    arc_dsc.center.x = cx;    // 圆心X
    arc_dsc.center.y = cy;    // 圆心Y
    // 外半径
    arc_dsc.radius = (radius - arc->GetUntouchableRadius())/2;
    // 厚度等于半径 → 实心填充
    arc_dsc.width = arc_dsc.radius;
    // 起始角度
    arc_dsc.start_angle = 0;
    // 终止角度
    arc_dsc.end_angle = 360;

    // 执行绘制
    lv_draw_arc(layer, &arc_dsc);
}

/**
 * @brief 事件处理
 * @param event 事件信息
 */
static void EventHandler(lv_event_t* event) {
    // 获取事件码
    lv_event_code_t code = lv_event_get_code(event);
    class Arc* arc = static_cast<class Arc*>(lv_event_get_user_data(event));
    // 获取图层指针
    lv_layer_t* layer = lv_event_get_layer(event);

    switch (code) {
    case LV_EVENT_DRAW_MAIN:
    {
        // 控件全局坐标
        const Rect rect = arc->GetGlobalGeometry();

        // 绘制背景
        DrawMain(layer, arc, rect);
        // 绘制进度条
        DrawIndicator(layer, arc, rect);
        // 绘制knob
        if (arc->IsKnobVisible()) {
            DrawKnob(layer, arc, rect);
        }
    }
    break;
    default:
        break;
    }
}

// 定义要监听的事件列表
static lv_event_code_t event_list[] = {
    // 绘制背景
    LV_EVENT_DRAW_MAIN,
};

Arc::Arc(std::string name, Widget* parent) : Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());

    // 绑定点击事件
    for (int i = 0; i < sizeof(event_list) / sizeof(event_list[0]); i++) {
        lv_obj_add_event_cb(m_object, EventHandler, event_list[i], this);
    }

    // 注册触摸回调
    AddTouchListener(this);
}

Arc::~Arc() {
    // 析构背景图片
    ImageIns::DestroyImageDes(m_image_main);
    // 析构进度图片
    ImageIns::DestroyImageDes(m_image_indicator);
    // 析构knob图片
    ImageIns::DestroyImageDes(m_image_knob);
}

void Arc::Create(ScreenLayer layer) {
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


void Arc::SetMainColor(const Color& color) {
    m_color_main = color;
}

Color Arc::GetMainColor() {
    return m_color_main;
}

void Arc::SetIndColor(const Color& color) {
    m_color_indicator = color;
}

Color Arc::GetIndColor() {
    return m_color_indicator;
}

void Arc::SetKnobColor(const Color& color) {
    m_color_knob = color;
}

Color Arc::GetKnobColor() {
    return m_color_knob;
}

void Arc::SetRadius(int radius) {
    if (0 > radius) {
        return;
    }
    m_radius = radius;
}

int Arc::GetRadius() {
    return m_radius;
}

void Arc::SetOpacity(int opacity) {
    if (0 > opacity || 100 < opacity) {
        return;
    }
    m_opacity = opacity;
}

int Arc::GetOpacity() {
    return m_opacity;
}

void Arc::SetStartAngle(int angle) {
    m_start_angle = angle;
}

int Arc::GetStartAngle() {
    return m_start_angle;
}

void Arc::SetRangeAngle(int angle) {
    m_range_angle = angle;
}

int Arc::GetRangeAngle() {
    return m_range_angle;
}

void Arc::SetMin(int value) {
    if (m_max <= value) {
        return;
    }
    m_min = value;
}

int Arc::GetMin() {
    return m_min;
}

void Arc::SetMax(int value) {
    if (value <= m_min) {
        return;
    }
    m_max = value;
}

int Arc::GetMax() {
    return m_max;
}

void Arc::SetProgress(int progress) {
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

int Arc::GetProgress() {
    return m_progress;
}

void Arc::SetUntouchableRadius(int radius) {
    m_untouchable_radius = radius;
}

int Arc::GetUntouchableRadius() {
    return m_untouchable_radius;
}

void Arc::SetPadding(int padding) {
    m_padding = padding;
}

int Arc::GetPadding() {
    return m_padding;
}

void Arc::SetImageMain(const std::string& path) {
    m_image_main = ImageIns::ParseImage(path);
}

ImageDes Arc::GetImageMain() {
    return m_image_main;
}

void Arc::SetImageIndicator(const std::string& path) {
    m_image_indicator = ImageIns::ParseImage(path);
}

ImageDes Arc::GetImageIndicator() {
    return m_image_indicator;
}

void Arc::SetImageKnob(const std::string& path) {
    m_image_knob = ImageIns::ParseImage(path);
}

ImageDes Arc::GetImageKnob() {
    return m_image_knob;
}

void Arc::SetKnobVisible(bool is_visible) {
    m_knob_visible = is_visible;
}

bool Arc::IsKnobVisible() {
    return m_knob_visible;
}

void Arc::AddProgressCallback(ProgressCallback callback) {
    m_progress_callback = callback;
}

void Arc::ExecuteProgressCallback(class Arc* slider, int progress) {
    if (m_progress_callback) {
        m_progress_callback(slider, progress);
    }
}

/**
 * @brief 校验触摸点是否超出中心禁止触摸圆，输出顺时针0~360°角度
 * @param centerX 圆心X坐标
 * @param centerY 圆心Y坐标
 * @param touchX 触摸点X坐标
 * @param touchY 触摸点Y坐标
 * @param forbidRadius 中心不可触摸半径
 * @param outAngleDeg 出参：顺时针角度 0 ~ 360.0°，X轴向右为0°，顺时针递增
 * @return false：触摸在禁止圆内；true：可触摸，角度有效
 */
static bool CheckTouchValid(double centerX, double centerY,
    double touchX, double touchY,
    double forbidRadius,
    double& outAngleDeg)
{
    const double PI = 3.141592653589793;

    double dx = touchX - centerX;
    double dy = touchY - centerY;

    double distSquare = dx * dx + dy * dy;
    double rSquare = forbidRadius * forbidRadius;

    if (distSquare <= rSquare)
    {
        return false;
    }

    // 屏幕Y向下，等价于数学Y=-dy
    double rad = std::atan2(-dy, dx);

    // 转换为顺时针 0~360°，X右为0，顺时针向下增大
    double clockDeg = fmod(-rad * 180.0 / PI + 360.0, 360.0);
    if (clockDeg < 0.0)
        clockDeg += 360.0;

    outAngleDeg = clockDeg;
    return true;
}

/**
* @brief 计算当前进度值
* @param arc 控件指针
* @param gesture 手势参数
* @return 返回进度值,-1代表计算失败
*/
static int CalculateProgress(class Arc* arc, const Gesture& gesture) {
    int progress = arc->GetProgress();

    const Rect rect = arc->GetGeometry();
    double angle{ 0.0 };
    if (CheckTouchValid(rect.m_width/2.0, rect.m_height/2.0, gesture.m_point.m_x, gesture.m_point.m_y,
        arc->GetUntouchableRadius(), angle)) {
        // 实际可滑动角度
        const double real_range = arc->GetStartAngle() + arc->GetRangeAngle();
        // 实际可滑动角度超过360°,如果当前触摸位置的角度小于起始角度则加上360°
        if (359 < real_range && angle < arc->GetStartAngle() - 10) {
            angle += 360;
        }

        // 触摸角度小于有效起始角度10°以内，则认为当前触摸位置位于起始角度
        if (angle < arc->GetStartAngle() && angle >= arc->GetStartAngle() - 10) {
            angle = arc->GetStartAngle();
        }
        // 触摸角度大于有效终止角度10°以内，则认为当前触摸位置位于终止角度
        else if (real_range < angle && real_range + 10 >= angle) {
            angle = real_range;
        }
        // 触摸角度在可触摸范围外
        if (arc->GetStartAngle() > angle || real_range < angle) {
            return progress;
        }

        progress = arc->GetMin() + (angle - arc->GetStartAngle()) / arc->GetRangeAngle() * (arc->GetMax() - arc->GetMin());
    }

    return progress;
}

bool Arc::Touch(Widget* widget, const Gesture& gesture) {
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
