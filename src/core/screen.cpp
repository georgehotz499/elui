#include "screen.h"

extern "C" {
#include "lvgl/lvgl.h"
#include "lvgl/src/display/lv_display.h"
}


/**
 * @brief 设置底层图层的基本属性
 */
static void SetLayerProperty(ScreenLayer object) {
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
    lv_obj_add_style(object, &style, LV_PART_MAIN);
}

Screen::Screen() {
    // 清除控件布局越界显示的滑条
    lv_obj_set_scrollbar_mode(lv_layer_bottom(), LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);
    // 禁止滚动
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(lv_layer_top(), LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(lv_layer_sys(), LV_SCROLLBAR_MODE_OFF);

    SetLayerProperty(lv_screen_active());
}

Screen* Screen::GetInstance() {
    static Screen ins;
    return &ins;
}

ScreenLayer Screen::GetBottomLayer() {
    return lv_layer_bottom();
}

ScreenLayer Screen::GetActiveLayer() {
    return lv_screen_active();
}

ScreenLayer Screen::GetTopLayer() {
    return lv_layer_top();
}

ScreenLayer Screen::GetSystemLayer() {
    return lv_layer_sys();
}
