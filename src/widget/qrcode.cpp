#include "qrcode.h"

extern "C" {
#include "lvgl/lvgl.h"
#include "lvgl/src/libs/qrcode/lv_qrcode.h"
}


QrCode::QrCode(std::string name, Widget* parent) :Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());
}

QrCode::~QrCode() {}

void QrCode::Create(ScreenLayer layer) {
    m_object = lv_qrcode_create(layer);
}

void QrCode::SetSize(int size) {
    lv_qrcode_set_size(m_object, size);
}

void QrCode::SetQrcodeColor(Color color) {
    lv_color_t qr_color;
    qr_color.red = color.m_r;
    qr_color.green = color.m_g;
    qr_color.blue = color.m_b;
    lv_qrcode_set_dark_color(m_object, qr_color);
}

void QrCode::SetQrBgColor(Color color) {
    lv_color_t qr_color;
    qr_color.red = color.m_r;
    qr_color.green = color.m_g;
    qr_color.blue = color.m_b;
    lv_qrcode_set_light_color(m_object, qr_color);
}

void QrCode::SetContent(std::string str) {
    lv_qrcode_update(m_object, str.c_str(), str.size());
}

void QrCode::SetGeometry(int x, int y, int w, int h) {
    Widget::SetGeometry(x, y, w, h);
    SetSize(w);
}

