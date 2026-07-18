#include "list.h"

#include "core/log.h"

extern "C" {
#include "lvgl/lvgl.h"
}


// 滑动触发阈值，超过此值视为列表滑动
#define SCROLL_THRESHOLD 10
// 滑动速度触发阈值，超过此值抬手后列表继续滚动
#define VELOCITY_THRESHOLD 100.0
// 滑动速度减速比
#define REDUCTION_RATIO 0.955
// 定时id
#define SCROLL_ID 1
// 单次滚动步进
#define SCROLL_STEP 0.02



List::List(std::string name, Widget* parent) :Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());
    // 注册触摸回调
    AddTouchListener(this);
}

List::~List() {

}

void List::Create(ScreenLayer layer) {
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

void List::SetCell(int row, int column) {
    if (1 > row || 1 > column) {
        LOGE("row %d or column less than 1.");
        return;
    }
    m_row = row;
    m_column = column;

    // 计算行列宽高
    Rect rect = GetGeometry();
    m_row_height = rect.m_height / m_row + m_row_span;
    m_column_width = rect.m_width / m_column + m_column_span;
}

int List::GetRowCount() {
    return m_row;
}

int List::GetColumnCount() {
    return m_column;
}

void List::SetRowSpan(int span) {
    m_row_span = span;

    // 计算行高
    Rect rect = GetGeometry();
    m_row_height = rect.m_height / m_row;
}

int List::GetRowSpan() {
    return m_row_span;
}

void List::SetColumnSpan(int span) {
    m_column_span = span;

    // 计算列宽
    Rect rect = GetGeometry();
    m_column_width = rect.m_width / m_column;
}

int List::GetColumnSpan() {
    return m_column_span;
}

void List::SetScrollDir(Dir dir) {
    m_scroll_dir = dir;
}

List::Dir List::GetScrollDir() {
    return m_scroll_dir;
}

void List::SetScrollCircular(bool circular) {
    m_scroll_circular = circular;
}

bool List::IsScrollCircular() {
    return m_scroll_circular;
}

int List::GetRowHeight() {
    return m_row_height;
}

int List::GetColumnWidth() {
    return m_column_width;
}

void List::SetCycle(bool is_cycle) {
    m_cycle = is_cycle;
}

bool List::GetCycle() {
    return m_cycle;
}

int List::GetFirstVisibleItemIndex() {
    return m_first_visible;
}

void List::AddItem(Button* btn, int index) {
    // 横向滑动
    if (kHor == m_scroll_dir) {
        // 列数
        const int column = index / m_row;

        // 行数
        const int row = index % m_row;

        // 设置控件位置
        btn->SetGeometry(column * m_column_width, row * m_row_height, m_column_width - m_column_span, m_row_height - m_row_span);
    }
    // 纵向滑动
    else {
        // 列数
        const int column = index % m_column;

        // 行数
        const int row = index / m_column;

        // 设置控件位置
        btn->SetGeometry(column * m_column_width, row * m_row_height, m_column_width - m_column_span, m_row_height - m_row_span);
    }

    // 控件添加到容器
    m_widget_vec.push_back(btn);

    // 刷新列表
    Refresh();
}

void List::AddGetListitemCountCallback(GetListCountCallback callback) {
    m_get_list_count_callback = callback;
}

void List::AddListUpdateCallback(ListUpdateCallback callback) {
    m_list_update_callback = callback;
}

void List::AddListitemClickedCallback(ListitemClickedCallback callback) {
    m_listitem_clocked_callback = callback;
}

int List::ExecuteGetListitemCountCallback() {
    if (!m_get_list_count_callback) {
        return 0;
    }
    return m_get_list_count_callback();
}

void List::ExecuteListUpdateCallback(Button* item, int index) {
    if (!m_list_update_callback) return;
    m_list_update_callback(item, index);
}

void List::ExecuteListitemClickedCallback(Button* item, int index) {
    if (!m_listitem_clocked_callback) return;
    m_listitem_clocked_callback(item, index);
}

void List::Refresh() {
    for (int level1 = 0; level1 < m_widget_vec.size(); ++level1) {
        ExecuteListUpdateCallback(m_widget_vec[level1], m_first_visible + level1);
    }
}

bool List::Touch(Widget* widget, const Gesture& gesture) {
    // 滑动(滑动动作很容易高频次触发，因此放在最前使得第一次就能命中)
    if (Gesture::kMoving == gesture.m_event) {
        m_vtracker.AddMovement(gesture.m_point.m_x, gesture.m_point.m_y, gesture.m_timestamp);
    }
    // 按下
    else if (Gesture::kPress == gesture.m_event) {
        // 初始化x滑动距离
        m_xdistance = 0;
        // 初始化y滑动距离
        m_ydistance = 0;
        // 清除滑动方向
        m_cur_scroll_dir = kNone;
        // 记录按下时手势
        m_gesture_press = gesture;
        // 标记按下时第一可见项索引
        m_first_visible_press = m_first_visible;
        // 标记按下时第一可见项位置
        m_first_visible_pos = m_widget_vec[0]->GetGeometry();
        // 清除所有点
        m_vtracker.Clear();
        m_vtracker.AddMovement(gesture.m_point.m_x, gesture.m_point.m_y, gesture.m_timestamp);
    }
    // 抬起
    else if (Gesture::kRelease == gesture.m_event) {
        m_vtracker.AddMovement(gesture.m_point.m_x, gesture.m_point.m_y, gesture.m_timestamp);
        // 计算抬起时的手速
        m_vtracker.ComputeCurrentVelocity(m_vx, m_vy);
    }

    // 获取子项数量
    const int item_count = ExecuteGetListitemCountCallback();
    // 滑动列表子项
    ScrollItem(item_count, gesture);
    // 点击列表子项
    ListitemClick(item_count, gesture);
    return false;
}

void List::ScrollItem(int item_count, const Gesture& gesture) {
    // 如果子项数量没超出列表显示范围，则列表不能滑动
    if (item_count <= m_row * m_column) {
        return;
    }

    // 滑动列表时计算滑动距离
    if (Gesture::kMoving == gesture.m_event || Gesture::kRelease == gesture.m_event) {
        m_xdistance = gesture.m_point.m_x - m_gesture_press.m_point.m_x;
        m_ydistance = gesture.m_point.m_y - m_gesture_press.m_point.m_y;
    }

    // 标记滑动方向
    if (kNone == m_cur_scroll_dir) {
        m_cur_scroll_dir = (SCROLL_THRESHOLD <= abs(m_xdistance)) ? kHor : (SCROLL_THRESHOLD <= abs(m_ydistance)) ? kVer : kNone;
    }
    
    // 横向滑动
    if (kHor == m_scroll_dir && kHor == m_cur_scroll_dir) {
        ScrollHorizontal(m_xdistance, item_count);
    }
    // 纵向滑动
    else if (kVer == m_scroll_dir && kVer == m_cur_scroll_dir) {
        ScrollVertical(m_ydistance, item_count);
    }

    // 启动定时器滚动子项
    if (Gesture::kRelease == gesture.m_event &&
        ((kHor == m_cur_scroll_dir && VELOCITY_THRESHOLD < abs(m_vx)) || (kVer == m_cur_scroll_dir && VELOCITY_THRESHOLD < abs(m_vy)))) {
        StartTimer(SCROLL_ID, 5);
    }
}

/**
 * @brief 计算横向滑动后的索引以及位置
 * @param rect 按下时第一项的位置 
 * @param item_index 按下时第一项索引 
 * @param column_width 列宽
 * @param distance 滑动距离
 * @param item_count 列表子项总数
 * @param row 列表行数
 * @param cycle 是否循环滚动
 * @param cal_index 计算后的第一可见项索引
 * @param cal_coordinate 计算后的第一可见项x位置
 */
static void CalculateHorizontal(const Rect& rect, const int item_index, const int column_width,
    const int distance, const int item_count, const int row, bool cycle, int& cal_index, int& cal_coordinate) {
    // 距离左边的临界距离
    const int left = column_width - abs(rect.m_x);
    // 距离右边的临界距离
    const int right = abs(rect.m_x);
    // 滑动后的x坐标
    const int x_coordinate = rect.m_x + distance;

    int distance_offset{ 0 };
    // 偏移的列数
    int item_offset{ 0 };
    // 右滑
    if (rect.m_x < x_coordinate) {
        // 计算是否滑动超过第一项边缘
        distance_offset = distance - right;
        if (0 < distance_offset) {
            item_offset = 1;
        }
        // 计算是否一次性滑动超过多列
        item_offset += distance_offset / column_width;
        cal_index = (item_index + item_count - (item_offset*row) % item_count) % item_count;
        cal_coordinate = x_coordinate % column_width;
        cal_coordinate = 0 < cal_coordinate ? -column_width + cal_coordinate : cal_coordinate;
    }
    // 左滑
    else {
        // 计算是否滑动超过最后一项边缘
        distance_offset = abs(distance) - left;
        if (0 < distance_offset) {
            item_offset = 1;
        }
        // 计算是否一次性滑动超过多列
        item_offset += distance_offset / column_width;
        cal_index = cycle? (item_index + item_offset * row) % item_count : item_index + item_offset * row;
        cal_coordinate = x_coordinate % column_width;
    }
}

bool List::ScrollHorizontal(const int x_distance, const int item_count) {
    // 滑动距离偏移
    int offset{ 0 };
    // 第一可见项
    int first_item{ 0 };
    // 是否滑动到端点，关闭定时器
    bool stop_timer{ false };
    CalculateHorizontal(m_first_visible_pos, m_first_visible_press, m_column_width, x_distance, item_count, m_row, m_cycle, first_item, offset);

    // 当不是循环滑动时，向右滑到顶部则不允许再滑动
    if (!m_cycle && 0 < x_distance && m_first_visible < first_item) {
        offset = 0;
        first_item = 0;
        stop_timer = true;
        //LOGI("move to start.");
    }
    // 当不是循环滑动时，向左滑到底部则不允许再滑动
    else if (!m_cycle && 0 > x_distance) {
        // 最后一项可见项
        int last_item = (item_count / m_row) * m_row + ((0 != item_count % m_row) ? m_row : 0) - m_row * m_column;
        // 子项数目小于可滑动数量时，第一可见项为0
        last_item = 0 > last_item ? 0 : last_item;
        if (last_item <= first_item) {
            offset = 0;
            first_item = last_item;
            stop_timer = true;
            //LOGI("move to end.");
        }
    }
    m_first_visible = first_item;

    // 移动控件位置
    AdjustItemPos(offset, item_count, m_cycle);

    return stop_timer;
}

/**
 * @brief 计算纵向滑动后的索引以及位置
 * @param rect 按下时第一项的位置
 * @param item_index 按下时第一项索引
 * @param row_height 行高
 * @param distance 滑动距离
 * @param item_count 列表子项总数
 * @param column 列表列数
 * @param cycle 是否循环滚动
 * @param cal_index 计算后的第一可见项索引
 * @param cal_coordinate 计算后的第一可见项y位置
 */
static void CalculateVertical(const Rect& rect, const int item_index, const int row_height,
    const int distance, const int item_count, const int column, bool cycle, int& cal_index, int& cal_coordinate) {
    // 距离顶部的临界距离
    const int top = row_height - abs(rect.m_y);
    // 距离底部的临界距离
    const int bottom = abs(rect.m_y);
    // 滑动后的y坐标
    const int y_coordinate = rect.m_y + distance;

    int distance_offset{ 0 };
    // 偏移的行数
    int item_offset{ 0 };
    // 下滑
    if (rect.m_y < y_coordinate) {
        // 计算是否滑动超过第一项边缘
        distance_offset = distance - bottom;
        if (0 < distance_offset) {
            item_offset = 1;
        }
        // 计算是否一次性滑动超过多列
        item_offset += distance_offset / row_height;
        cal_index = (item_index + item_count - (item_offset * column) % item_count) % item_count;
        cal_coordinate = y_coordinate % row_height;
        cal_coordinate = 0 < cal_coordinate ? -row_height + cal_coordinate : cal_coordinate;
    }
    // 上滑
    else {
        // 计算是否滑动超过最后一项边缘
        distance_offset = abs(distance) - top;
        if (0 < distance_offset) {
            item_offset = 1;
        }
        // 计算是否一次性滑动超过多列
        item_offset += distance_offset / row_height;
        cal_index = cycle? (item_index + item_offset * column) % item_count : item_index + item_offset * column;
        cal_coordinate = y_coordinate % row_height;
    }
}

bool List::ScrollVertical(const int y_distance, const int item_count) {
    // 滑动距离偏移
    int offset{ 0 };
    // 第一可见项
    int first_item{ 0 };
    // 是否滑动到端点，关闭定时器
    bool stop_timer{ false };
    CalculateVertical(m_first_visible_pos, m_first_visible_press, m_row_height, y_distance, item_count, m_column, m_cycle, first_item, offset);

    // 当不是循环滑动时，向下滑到顶部则不允许再滑动
    if (!m_cycle && 0 < y_distance && m_first_visible < first_item) {
        offset = 0;
        first_item = 0;
        stop_timer = true;
        //LOGI("move to start.");
    }
    // 当不是循环滑动时，向上滑到底部则不允许再滑动
    else if (!m_cycle && 0 > y_distance) {
        // 最后一项可见项
        int last_item = (item_count / m_column) * m_column + ((0 != item_count % m_column)? m_column:0) - m_row * m_column;
        // 子项数目小于可滑动数量时，第一可见项为0
        last_item = 0 > last_item ? 0 : last_item;
        if (last_item <= first_item) {
            offset = 0;
            first_item = last_item;
            stop_timer = true;
            //LOGI("move to end.");
        }
    }
    m_first_visible = first_item;
    
    // 移动控件位置
    AdjustItemPos(offset, item_count, m_cycle);
    return stop_timer;
}

void List::AdjustItemPos(const int offset, const int item_count, const bool is_cycle) {
    int column{ 0 };
    int row{ 0 };
    const int widget_size = m_widget_vec.size();

    // 横向滑动
    if (kHor == m_scroll_dir) {
        for (int index = 0; index < widget_size; ++index) {
            // 列数
            column = index / m_row;

            // 行数
            row = index % m_row;

            // 设置控件位置
            m_widget_vec[index]->SetGeometry(column * m_column_width + offset, row * m_row_height, m_column_width - m_column_span, m_row_height - m_row_span);
            // 非滚动模式
            if (!is_cycle && m_first_visible + index >= item_count) {
                m_widget_vec[index]->Hide();
                continue;
            }
            m_widget_vec[index]->Show();
            ExecuteListUpdateCallback(m_widget_vec[index], (m_first_visible + index) % item_count);
        }
    }
    // 纵向滑动
    else {
        for (int index = 0; index < widget_size; ++index) {
            // 列数
            column = index % m_column;

            // 行数
            row = index / m_column;

            // 设置控件位置
            m_widget_vec[index]->SetGeometry(column * m_column_width, row * m_row_height + offset, m_column_width - m_column_span, m_row_height - m_row_span);
            // 非滚动模式
            if (!is_cycle && m_first_visible + index >= item_count) {
                m_widget_vec[index]->Hide();
                continue;
            }
            m_widget_vec[index]->Show();
            ExecuteListUpdateCallback(m_widget_vec[index], (m_first_visible + index) % item_count);
        }
    }
}

void List::ExecuteTimerCallback(int timer_id) {
    switch (timer_id) {
    case SCROLL_ID:
    {
        // 横向滑动
        if (kHor == m_cur_scroll_dir && VELOCITY_THRESHOLD < abs(m_vx)) {
            m_xdistance += static_cast<int>(m_vx * SCROLL_STEP);
            // 减速
            m_vx *= REDUCTION_RATIO;
            // 滚动子项
            if (ScrollHorizontal(m_xdistance, ExecuteGetListitemCountCallback())) {
                break;
            }
        }
        // 纵向滑动
        else if (kVer == m_cur_scroll_dir && VELOCITY_THRESHOLD < abs(m_vy)) {
            m_ydistance += static_cast<int>(m_vy * SCROLL_STEP);
            // 减速
            m_vy *= REDUCTION_RATIO;
            // 滚动子项
            if (ScrollVertical(m_ydistance, ExecuteGetListitemCountCallback())) {
                break;
            }
        }
        else {
            break;
        }
        return;
    }
        break;
    default:
        break;
    }
    // 停止定时器
    StopTimer(timer_id);
}

/**
 * @brief 判断是否按压中子项
 * @param widget_vec 列表子项集合
 * @param first_item 第一可见项
 * @param press_item 触摸的子项索引
 * @param gesture 手势
 */
static void PressItem(std::vector<Button*>& widget_vec, int first_item, int& press_item, const Gesture& gesture) {
    for (int level1 = 0; level1 < widget_vec.size(); ++level1) {
        if (widget_vec[level1]->InRange(gesture.m_point)) {
            widget_vec[level1]->SetStatusPress();
            press_item = first_item + level1;
            return;
        }
    }
}

/**
 * @brief 释放按压控件
 * @param widget_vec 列表子项集合
 */
static void ReleaseItem(std::vector<Button*>& widget_vec) {
    for (auto& iter : widget_vec) {
        iter->SetStatusRelease();
    }
}

void List::ListitemClick(int item_count, const Gesture& gesture) {
    // 滑动(滑动动作很容易高频次触发，因此放在最前使得第一次就能命中)
    if (Gesture::kMoving == gesture.m_event && -1 != m_touch_item && kNone != m_scroll_dir) {
        ReleaseItem(m_widget_vec);
        m_touch_item = -1;
    }
    // 按下
    else if (Gesture::kPress == gesture.m_event) {
        m_touch_item = -1;
        PressItem(m_widget_vec, m_first_visible_press, m_touch_item, gesture);
    }
    // 抬起
    else if (Gesture::kRelease == gesture.m_event && -1 != m_touch_item) {
        ReleaseItem(m_widget_vec);
        // 执行点击回调
        ExecuteListitemClickedCallback(m_widget_vec[m_touch_item % m_widget_vec.size()], m_touch_item % item_count);
        m_touch_item = -1;
    }
}
