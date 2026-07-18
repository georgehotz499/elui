#pragma once
#include "object.h"

#include <string>

#include "core/color.h"
#include "core/screen.h"
#include "core/rect.h"
#include "core/point.h"


class Widget;
// 控件长按监听
class LongPressListener {
public:
    /**
     * @brief 长按回调
     * @param widget 控件指针
     */
    virtual void LongPress(Widget* widget) = 0;
};

// 手势事件
class Gesture {
public:
    enum Event {
        kNone,
        kPress,     // 按下
        kMoving,    // 滑动
        kRelease    // 松开
    };

    // 当前事件
    Event m_event{ kNone };
    // 时间戳
    int64_t m_timestamp{ 0 };
    // 坐标
    Point m_point{ 0, 0 };
};

// 控件触摸监听
class TouchListener {
public:
    /**
    * @brief 触摸回调
    * @param widget 控件指针
    * @param gesture 手势
    * @return true 拦截事件传递；false 事件继续传递
    */
    virtual bool Touch(Widget* widget, const Gesture& gesture) = 0;
};

class Widget : public Object {
public:
    explicit Widget(std::string name, Widget* parent);
    ~Widget();

    /**
    * @brief 设置控件位置
    */
    void SetPos(int x, int y);

    /**
    * @brief 设置控件大小
    */
    void SetSize(int w, int h);

    /**
    * @brief 设置控件位置大小
    */
    void SetGeometry(int x, int y, int w, int h);

    /**
    * @brief 获取控件大小
    */
    Rect GetGeometry();

    /**
    * @brief 获取控件全局坐标
    */
    Rect GetGlobalGeometry();

    /**
    * @brief 设置背景颜色
    * @param color ARGB颜色值
    */
    void SetBgColor(const Color& color);

    /**
    * @brief 设置背景色不透明度
    * @param opacity 0:全透明；100：不透明
    */
    void SetBgOpacity(int opacity);

    /**
    * @brief 设置对象圆角
    * @param radius 圆角半径
    */
    void SetRadius(int radius);

    /**
    * @brief 显示控件
    */
    void Show();

    /**
    * @brief 隐藏控件
    */
    void Hide();

    /**
    * @brief 获取对象图层
    */
    ScreenLayer GetLayer();

    /**
    * @brief 获取对象图层的上一级图层
    */
    ScreenLayer GetParentLayer();

    /**
     * @brief 创建LVGL对象
     * @return 返回LVGL对象指针
     */
    virtual void Create(ScreenLayer layer) = 0;

    /**
     * @brief 销毁对象
     */
    void Destroy();

    /**
     * @brief 将控件移动到父控件容器的顶层
     */
    virtual void MoveToTop();

    /**
     * @brief 将控件移动到父控件容器的底层
     */
    virtual void MoveToBottom();

    /**
     * @brief 获取子控件
     */
    std::list<Widget*> GetWidgets();

    /**
     * @brief 刷新控件
     */
    void Refresh();

    /**
     * @brief 设置触摸穿透
     * @param touchable true 可触摸；false 不可触摸
     */
    void SetTouchable(bool touchable);

    /**
     * @brief 注册长按监听
     */
    void AddLongPressListener(LongPressListener* listener);

    /**
     * @brief 执行长按监听
     */
    void ExecuteLongListener();

    /**
     * @brief 注册触摸监听
     */
    void AddTouchListener(TouchListener* listener);

    /**
     * @brief 执行触摸监听
     */
    void ExecuteTouchListener(void* lv_event);

    /**
     * @brief 计算控件点击位置
     * @param screen_pos 点击的屏幕位置
     * @return 换算后对应的控件位置
     */
    Point TransformPosition(const Point& screen_pos);

    /**
     * @brief 当前坐标是否在控件范围内
     * @param point 坐标点
     * @return true 坐标点在范围内；false 坐标点不在范围内
     */
    bool InRange(const Point& point);

private:
    /**
     * @brief 注册LVGL控件删除回调事件（不需要手动执行注销，LVGL会自动清理）
     */
    void AddDeleteEvent();

protected:
    // LVGL对象（每个对象其实也就是一个图层）
    ScreenLayer m_object{ nullptr };

private:
    // 对子类屏蔽Object类成员
    using Object::m_parent;
    using Object::m_children;
    // 控件几何属性
    Rect m_rect;
    // 控件背景颜色
    void* m_style{ nullptr };
    // 长按监听
    LongPressListener* m_long_press{ nullptr };
    // 触摸监听
    TouchListener* m_touch{ nullptr };
    // 手势
    Gesture m_gesture;
};
