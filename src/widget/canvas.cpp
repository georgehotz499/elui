#include "canvas.h"

extern "C" {
#include "lvgl/lvgl.h"
}


Canvas::Canvas(std::string name, Widget* parent) :Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());
}

Canvas::~Canvas() {
    if (m_canvas_buf) {
        delete[] m_canvas_buf;
    }
}

void Canvas::Create(ScreenLayer layer) {
    m_object = lv_canvas_create(layer);
}

void Canvas::SetGeometry(int x, int y, int w, int h) {
    // 析构先前buffer
    if (m_canvas_buf) { delete[] m_canvas_buf; }
    // ARGB图像数据，所以最后乘4
    const int buf_size = w * h * 4;
    m_canvas_buf = new uint8_t[buf_size];
    memset(m_canvas_buf, 0X00, buf_size);
    // 设置buffer
    lv_canvas_set_buffer(m_object, m_canvas_buf, w, h, LV_COLOR_FORMAT_ARGB8888);

    // 设置控件几何属性
    Widget::SetGeometry(x, y, w, h);
}

void Canvas::Clear() {
    lv_canvas_fill_bg(m_object, lv_color_hex3(0xFFFFFF), LV_OPA_TRANSP);
}

void Canvas::DrawRect(int x, int y, int w, int h, uint32_t color, int radius) {
    lv_layer_t layer;
    lv_canvas_init_layer(m_object, &layer);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);

    dsc.bg_color.red = (uint8_t)(color >> 16);
    dsc.bg_color.green = (uint8_t)(color >> 8);
    dsc.bg_color.blue = (uint8_t)(color);

    dsc.radius = radius;

    lv_area_t coords = { x, y, x+w-1, y+h-1 };

    lv_draw_rect(&layer, &dsc, &coords);

    lv_canvas_finish_layer(m_object, &layer);

    // 刷新画布显示
    Refresh();
}

void Canvas::DrawArc(int center_x, int center_y, int radius, int border_width, int start_angle, int end_angle, uint32_t color) {
    lv_layer_t layer;
    lv_canvas_init_layer(m_object, &layer);

    lv_draw_arc_dsc_t dsc;
    lv_draw_arc_dsc_init(&dsc);
    dsc.color = lv_color_hex3(color);
    dsc.center.x = center_x;
    dsc.center.y = center_y;
    dsc.radius = radius;
    dsc.width = border_width;
    dsc.start_angle = start_angle;
    dsc.end_angle = end_angle;

    lv_draw_arc(&layer, &dsc);

    lv_canvas_finish_layer(m_object, &layer);

    // 刷新画布显示
    Refresh();
}

void Canvas::DrawLine(int x1, int y1, int x2, int y2, int width, uint32_t color, int radius) {
    lv_layer_t layer;
    lv_canvas_init_layer(m_object, &layer);

    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = lv_color_hex3(color);
    dsc.width = width;
    dsc.round_end = radius;
    dsc.round_start = radius;
    dsc.p1.x = x1;
    dsc.p1.y = y1;
    dsc.p2.x = x2;
    dsc.p2.y = y2;
    lv_draw_line(&layer, &dsc);

    lv_canvas_finish_layer(m_object, &layer);

    // 刷新画布显示
    Refresh();
}
