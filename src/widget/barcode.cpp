#include "barcode.h"
#include "core/log.h"

extern "C" {
#include "lvgl/lvgl.h"
#include "lvgl/src/libs/barcode/lv_barcode.h"
}


BarCode::BarCode(std::string name, Widget* parent) :Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());
}

BarCode::~BarCode() {}

void BarCode::Create(ScreenLayer layer) {
    m_object = lv_barcode_create(layer);
}

void BarCode::SetQrcodeColor(Color color) {
    lv_color_t qr_color;
    qr_color.red = color.m_r;
    qr_color.green = color.m_g;
    qr_color.blue = color.m_b;
    lv_barcode_set_dark_color(m_object, qr_color);
}

void BarCode::SetQrBgColor(Color color) {
    lv_color_t qr_color;
    qr_color.red = color.m_r;
    qr_color.green = color.m_g;
    qr_color.blue = color.m_b;
    lv_barcode_set_light_color(m_object, qr_color);
}

void BarCode::SetContent(std::string str) {
    lv_barcode_update(m_object, str.c_str());
}

void BarCode::SetScale(uint16_t scale) {
    if (scale < 1) {
        LOGW("Scale must greater than 1.");
        return;
    }
    lv_barcode_set_scale(m_object, scale);
}

void BarCode::SetDirection(int direction) {
    if (0 == direction) {
        lv_barcode_set_direction(m_object, LV_DIR_HOR);
    }
    else if (1 == direction) {
        lv_barcode_set_direction(m_object, LV_DIR_VER);
    }
}

int BarCode::GetWidth() {
    return lv_obj_get_width(m_object);
}

int BarCode::GetHeight() {
    return lv_obj_get_height(m_object);
}

