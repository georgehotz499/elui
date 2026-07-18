#include "widget.h"
#include "layer.h"

#include "core/log.h"
#include "core/screen.h"
#include "core/http_client.h"

extern "C" {
#include "lvgl/lvgl.h"
}

#include <queue>


Widget::Widget(std::string name, Widget* parent) :Object(name, parent) {

}

Widget::~Widget() {
    if (m_style) {
        delete (lv_style_t*)m_style;
    }
}

void Widget::SetPos(int x, int y) {
    m_rect.m_x = x;
    m_rect.m_y = y;
    lv_obj_set_pos(m_object, x, y);
}

void Widget::SetSize(int w, int h) {
    m_rect.m_width = w;
    m_rect.m_height = h;
    lv_obj_set_size(m_object, w, h);
}

void Widget::SetGeometry(int x, int y, int w, int h) {
    m_rect.m_x = x;
    m_rect.m_y = y;
    m_rect.m_width = w;
    m_rect.m_height = h;
    lv_obj_set_pos(m_object, x, y);
    lv_obj_set_size(m_object, w, h);
}

Rect Widget::GetGeometry() {
    //Rect rect;
    //rect.m_x = lv_obj_get_x(m_object);
    //rect.m_y = lv_obj_get_y(m_object);
    //rect.m_width = lv_obj_get_width(m_object);
    //rect.m_height = lv_obj_get_height(m_object);

    //return rect;

    return m_rect;
}

Rect Widget::GetGlobalGeometry() {
    // x坐标
    int x{ 0 };
    // y坐标
    int y{ 0 };

    // 遍历到顶层控件
    Rect rect;
    Widget* widget = this;
    do {
        rect = widget->GetGeometry();
        x += rect.m_x;
        y += rect.m_y;
    } while (widget = dynamic_cast<Widget*>(widget->GetParent()));

    return {x, y, m_rect.m_width, m_rect.m_height };
}

void Widget::SetBgColor(const Color& color) {
    if (!m_style) {
        m_style = new lv_style_t;
    }
    lv_style_init((lv_style_t*)m_style);
    lv_color_t lv_color;
    lv_color.red = color.m_r;
    lv_color.green = color.m_g;
    lv_color.blue = color.m_b;
    // 设置背景颜色（例如：红色 #FF0000）
    lv_style_set_bg_color((lv_style_t*)m_style, lv_color);
    // 必须设置不透明度（0-255），否则背景透明不可见
    uint8_t opacity_level = color.m_a;
    lv_style_set_bg_opa((lv_style_t*)m_style, opacity_level);
    // 将样式应用到标签（LV_PART_MAIN 表示标签主体部分）
    lv_obj_add_style(m_object, (lv_style_t*)m_style, LV_PART_MAIN);
}

void Widget::SetBgOpacity(int opacity) {
    uint8_t opacity_level = (opacity % 101) * 2.55;
    lv_obj_set_style_bg_opa(m_object, opacity_level, LV_PART_MAIN);
}

void Widget::SetRadius(int radius) {
    lv_obj_set_style_radius(m_object, radius, LV_STATE_DEFAULT);
}

void Widget::Show() {
    lv_obj_clear_flag(m_object, LV_OBJ_FLAG_HIDDEN);
}

void Widget::Hide() {
    lv_obj_add_flag(m_object, LV_OBJ_FLAG_HIDDEN);
}

ScreenLayer Widget::GetLayer() {
    return m_object;
}

ScreenLayer Widget::GetParentLayer() {
    return lv_obj_get_parent(m_object);
}

void Widget::Destroy() {
    // 注册对象析构回调，在回调中delete对象自身
    AddDeleteEvent();
    // 隐藏自身对象
    Hide();
    // 从父对象中移除自己
    RemoveFromParent();
    // 关闭定时器
    StopAllTimer();

    // 调用子对象的Destroy，递归销毁子对象
    std::queue<Widget*> widgets;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        Widget* widget = dynamic_cast<Widget*>(*it);
        if (widget) {
            // 保存对象指针，不能直接在此递归销毁对象
            widgets.push(widget);
        }
    }
    while (!widgets.empty()) {
        widgets.front()->Destroy();
        widgets.pop();
    }
    // 同步删除LVGL对象
    if (m_object) {
        lv_obj_del(m_object);
    }
}

void Widget::MoveToTop() {
    Object::MoveToTop();
    // 层级结构刷新
    LAYER_MGR->UpdateLayer();
}

void Widget::MoveToBottom() {
    Object::MoveToBottom();
    // 层级结构刷新
    LAYER_MGR->UpdateLayer();
}

std::list<Widget*> Widget::GetWidgets() {
    std::list<Widget*> widgets;
    for (auto object : m_children) {
        auto widget = dynamic_cast<Widget*>(object);
        if (!widget) {
            continue;
        }
        widgets.push_back(widget);
    }

    return widgets;
}

void Widget::Refresh() {
    // 刷新控件
    lv_obj_invalidate(m_object);
}

void Widget::SetTouchable(bool touchable) {
    if (touchable) {
        lv_obj_add_flag(m_object, LV_OBJ_FLAG_CLICKABLE);
    }
    else if (!touchable) {
        // 设置触摸无效
        lv_obj_remove_flag(m_object, LV_OBJ_FLAG_CLICKABLE);
    }
}

/**
 * @brief 长按事件回调
 * @param event 事件信息
 */
static void LongPressEventHandle(lv_event_t* event) {
    Widget* btn = static_cast<Widget*>(lv_event_get_user_data(event));
    btn->ExecuteLongListener();
}

void Widget::AddLongPressListener(LongPressListener* listener) {
    m_long_press = listener;
    // 注册回调事件
    if (listener) {
        lv_obj_add_event_cb(m_object, LongPressEventHandle, LV_EVENT_LONG_PRESSED, this);
        lv_obj_add_event_cb(m_object, LongPressEventHandle, LV_EVENT_LONG_PRESSED_REPEAT, this);
    }
    // 注销回调事件
    else {
        lv_obj_remove_event_cb(m_object, LongPressEventHandle);
    }
}

void Widget::ExecuteLongListener() {
    if (m_long_press) {
        m_long_press->LongPress(this);
    }
}

/**
 * @brief 控件触摸回调
 * @param event 事件信息
 */
static void TouchEventHandle(lv_event_t* event) {
    Widget* widget = static_cast<Widget*>(lv_event_get_user_data(event));
    widget->ExecuteTouchListener(event);
}

void Widget::AddTouchListener(TouchListener* listener) {
    m_touch = listener;
    // 注册回调事件
    if (m_touch) {
        lv_obj_add_event_cb(m_object, TouchEventHandle, LV_EVENT_PRESSED, this);
        lv_obj_add_event_cb(m_object, TouchEventHandle, LV_EVENT_PRESSING, this);
        lv_obj_add_event_cb(m_object, TouchEventHandle, LV_EVENT_RELEASED, this);
    }
    // 注销回调事件
    else {
        lv_obj_remove_event_cb(m_object, TouchEventHandle);
    }
}

void Widget::ExecuteTouchListener(void* lv_event) {
    lv_event_t* event = static_cast<lv_event_t*>(lv_event);
    lv_event_code_t code = lv_event_get_code(event);

    // 获取当前活动的输入设备
    lv_indev_t* indev = lv_event_get_indev(event);
    if (!indev) {
        return;
    }
    lv_point_t screen_point{ 0, 0 };

    // 从输入设备获取全局（屏幕）坐标
    lv_indev_get_point(indev, &screen_point);

    // 将全局坐标转换为相对于控件的本地坐标
    const Point local_point = TransformPosition({ screen_point.x, screen_point.y });

    // 拦截事件上报
    bool stop_bubble = false;
    // 滑动
    if (LV_EVENT_PRESSING == code) {
        // 按压就会上报，过滤掉重复按压上报坐标
        if (local_point == m_gesture.m_point) {
            return;
        }
        m_gesture.m_event = Gesture::kMoving;
        m_gesture.m_timestamp = Http::GetMillisecondTimestamp();
        m_gesture.m_point = local_point;

        stop_bubble = m_touch->Touch(this, m_gesture);
    }
    else {
        switch (code) {
        case LV_EVENT_PRESSED: // 按压
            m_gesture.m_event = Gesture::kPress;
            m_gesture.m_timestamp = Http::GetMillisecondTimestamp();
            m_gesture.m_point = local_point;

            stop_bubble = m_touch->Touch(this, m_gesture);
            break;
        case LV_EVENT_RELEASED: // 抬起
            m_gesture.m_event = Gesture::kRelease;

            m_gesture.m_timestamp = Http::GetMillisecondTimestamp();
            m_gesture.m_point = local_point;

            stop_bubble = m_touch->Touch(this, m_gesture);
            break;
        }
    }

    // 拦截事件上报
    if (stop_bubble) {
        // 拦截事件向上传递
        lv_event_stop_bubbling(event);
    }
}

Point Widget::TransformPosition(const Point& screen_pos) {
    // x坐标
    int x{ 0 };
    // y坐标
    int y{ 0 };

    // 遍历到顶层控件
    Rect rect;
    Widget* widget = this;
    do {
        rect = widget->GetGeometry();
        x += rect.m_x;
        y += rect.m_y;
    } while (widget = dynamic_cast<Widget*>(widget->GetParent()));

    return Point(screen_pos.m_x - x, screen_pos.m_y - y);
}

bool Widget::InRange(const Point& point) {
    return ((m_rect.m_x < point.m_x && (point.m_x < m_rect.m_x + m_rect.m_width)) &&
        (m_rect.m_y < point.m_y && (point.m_y < m_rect.m_y + m_rect.m_height)));
}

/**
 * @brief 删除LVGL控件回调函数
 * @param e 事件参数
 */
static void ObjectDeleteEvent(lv_event_t* e) {
    // 获取用户数据
    Widget* widget = (Widget*)lv_event_get_user_data(e);
    LOGI("ObjectDeleteEvent delete widget:%s", widget->GetName().c_str());
    // 析构对象数据
    delete widget;
}

void Widget::AddDeleteEvent() {
    lv_obj_add_event_cb(m_object, ObjectDeleteEvent, LV_EVENT_DELETE, this);
}
