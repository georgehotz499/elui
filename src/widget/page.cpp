#include "page.h"
#include "core/log.h"
#include "layer.h"

extern "C" {
#include "lvgl/lvgl.h"
}


Page::Page(std::string name, Widget* parent, ScreenLayer layer) :Widget(name, parent) {
    // 创建对象
    if (parent) {
        Create(parent->GetLayer());
    }
    else {
        Create(layer);
    }
}

Page::~Page() {
    // 将页面移出图层管理
    LAYER_MGR->RemovePage(this, static_cast<Layer::Level>(m_layer));
}

void Page::Create(ScreenLayer layer) {
    // 创建对象
    m_object = lv_obj_create(layer);
    // 清除控件布局越界显示的滑条
    lv_obj_set_scrollbar_mode(m_object, LV_SCROLLBAR_MODE_OFF);

    // 初始化样式并设置背景属性
    static lv_style_t style;
    lv_style_init(&style);

    // 设置背景不透明度（0-255，255表示完全不透明）
    lv_style_set_bg_opa(&style, LV_OPA_0);

    // 可选：设置内边距（文本与背景边框的距离）
    lv_style_set_pad_all(&style, 0);

    // 将边框宽度设为0以取消边框
    lv_style_set_border_width(&style, 0);

    // 可选：设置圆角
    lv_style_set_radius(&style, 0);

    // 3. 将样式应用到标签（LV_PART_MAIN 表示标签主体部分）
    lv_obj_add_style(m_object, &style, LV_PART_MAIN);

    // 将页面添加至图层管理
    LAYER_MGR->AddPage(this, static_cast<Layer::Level>(m_layer));
    LAYER_MGR->UpdateLayer();
}

int Page::GetLevel() {
    return m_layer;
}

void Page::MoveToTop() {
    LAYER_MGR->MoveToTop(this);
    LAYER_MGR->UpdateLayer();
}

void Page::MoveToBottom() {
    LAYER_MGR->MoveToBottom(this);
    LAYER_MGR->UpdateLayer();
}

void Page::MoveLayer(int level) {
    LAYER_MGR->MoveLayer(this, static_cast<Layer::Level>(level));
    // 同步图层信息
    m_layer = level;
    LAYER_MGR->UpdateLayer();
}

void Page::AddTimeoutCallback(TimeoutCallback callback) {
    m_timeout_fun = callback;
}

void Page::ExecuteTimerCallback(int timer_id) {
    // 执行定时器回调函数
    if (m_timeout_fun && m_timeout_fun(timer_id)) {
        // 返回true，关闭定时器
        StopTimer(timer_id);
    }
}
