#pragma once
#include "widget.h"
#include "button.h"

#include "core/velocity_tracker.h"

#include <vector>
#include <functional>


class List : public Widget, protected TouchListener {
public:
    // 滑动方向
    enum Dir {
        kNone,
        kVer,   // 垂直方向
        kHor,   // 水平方向
    };

private:
    // 获取列表子项总数回调
    using GetListCountCallback = std::function<int (void)>;

    // 列表刷新回调
    using ListUpdateCallback = std::function<void (Button*, int)>;

    // 列表子项点击回调
    using ListitemClickedCallback = std::function<void (Button*, int)>;

public:
    List(std::string name, Widget* parent);
    ~List();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
    * @brief 设置列表行列数
    * @param row 行数
    * @param column 列数
    */
    void SetCell(int row, int column);

    /**
    * @brief 获取行数
    */
    int GetRowCount();

    /**
    * @brief 获取列数
    */
    int GetColumnCount();

    /**
     * @brief 设置行间距
     * @param span 间距
     */
    void SetRowSpan(int span);

    /**
    * @brief 获取行间距
    */
    int GetRowSpan();

    /**
     * @brief 设置列间距
     * @param span 间距
     */
    void SetColumnSpan(int span);

    /**
     * @brief 获取列间距
     */
    int GetColumnSpan();

    /**
     * @brief 设置列表滚动方向
     */
    void SetScrollDir(Dir dir);

    /**
     * @brief 获取列表滚动方向
     */
    Dir GetScrollDir();

    /**
     * @brief 设置是否循环滚动
     */
    void SetScrollCircular(bool circular);

    /**
     * @brief 获取当前滚动模式
     * @return true 循环滚动；false 单向滚动
     */
    bool IsScrollCircular();

    /**
     * @brief 获取行高
     */
    int GetRowHeight();

    /**
     * @brief 获取列高
     */
    int GetColumnWidth();

    /**
    * @brief 设置循环滚动
    */
    void SetCycle(bool is_cycle);

    /*
    * @brief 获取是否循环滚动
    */
    bool GetCycle();

    /**
    * @brief 获取第一可见项索引
    */
    int GetFirstVisibleItemIndex();

    /**
     * @brief 添加按键至列表（注意只能添加按键控件）
     * @param btn 按键控件指针
     * @param index 当前控件索引
     */
    void AddItem(Button* btn, int index);

    /**
     * @brief 注册获取列表子项数回调
     */
    void AddGetListitemCountCallback(GetListCountCallback callback);

    /**
     * @brief 注册列表刷新回调
     */
    void AddListUpdateCallback(ListUpdateCallback callback);

    /**
     * @brief 注册列表子项点击回调
     */
    void AddListitemClickedCallback(ListitemClickedCallback callback);

    /**
     * @brief 刷新控件
     */
    void Refresh();

private:
    /**
     * @brief 执行获取列表子项数回调
     */
    int ExecuteGetListitemCountCallback();

    /**
    * @brief 执行列表刷新回调
    */
    void ExecuteListUpdateCallback(Button* item, int index);

    /**
    * @brief 执行列表子项点击回调
    */
    void ExecuteListitemClickedCallback(Button* item, int index);

    /**
    * @brief 触摸回调
    * @param widget 控件指针
    * @param gesture 手势
    * @return true 拦截事件传递；false 事件继续传递
    */
    bool Touch(Widget* widget, const Gesture& gesture) override;

    /**
     * @brief 列表控件滑动
     * @param item_count 子项总数
     * @param gesture 手势
     */
    void ScrollItem(int item_count, const Gesture& gesture);

    /**
    * @brief 横向滚动
    * @param x_distance 横向滑动距离
    * @param item_count 子项数目
    * return true 滑动到端点，关闭定时器
    */
    bool ScrollHorizontal(const int x_distance, const int item_count);

    /**
    * @brief 纵向滚动
    * @param y_distance 纵向滑动距离
     * @param item_count 子项数目
     * return true 滑动到端点，关闭定时器
    */
    bool ScrollVertical(const int y_distance, const int item_count);

    /**
     * @brief 调整列表子项位置
     * @param offset 位置偏移
     * @param item_count 列表子项总数
     * @param is_cycle 是否循环滚动
     */
    void AdjustItemPos(const int offset, const int item_count, const bool is_cycle);

    /**
    * @brief 执行定时器回调
    * @param timer_id 定时器id
    */
    void ExecuteTimerCallback(int timer_id) override;

    /**
    * @brief 点击列表子项
    * @param item_count 子项总数
    * @param gesture 手势
    */
    void ListitemClick(int item_count, const Gesture& gesture);

private:
    // 行数
    int m_row{ 1 };
    // 列数
    int m_column{ 1 };
    // 行间距
    int m_row_span{ 0 };
    // 列间距
    int m_column_span{ 0 };
    // 列表滚动方向
    Dir m_scroll_dir{ kHor };
    // 是否支持循环滚动
    bool m_scroll_circular{ false };
    // 行高
    int m_row_height{ 0 };
    // 列宽
    int m_column_width{ 0 };
    // 是否循环滚动
    bool m_cycle{ false };
    // 存储列表控件
    std::vector<Button*> m_widget_vec;
    // 按下时手势
    Gesture m_gesture_press;
    // 手速计算
    VelocityTracker m_vtracker;
    // 抬起时的手速
    float m_vx{ 0.0 };
    float m_vy{ 0.0 };
    // 标记当前的滑动方向
    Dir m_cur_scroll_dir{ kNone };
    // 当前的第一可见项索引
    int m_first_visible{ 0 };
    // 按下时第一可见项索引
    int m_first_visible_press{ 0 };
    // 按下时第一可见项位置
    Rect m_first_visible_pos;
    // 获取列表子项总数回调
    GetListCountCallback m_get_list_count_callback{ nullptr };
    // 列表刷新回调
    ListUpdateCallback m_list_update_callback{ nullptr };
    // 列表子项点击回调
    ListitemClickedCallback m_listitem_clocked_callback{ nullptr };
    // x滑动距离
    int m_xdistance{ 0 };
    // y滑动距离
    int m_ydistance{ 0 };
    // 手指触摸的列表子项索引(-1代表当前未触摸子项)
    int m_touch_item{ -1 };
};
