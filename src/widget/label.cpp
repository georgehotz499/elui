#include "label.h"

extern "C" {
#include "lvgl/lvgl.h"
}

#include "core/resources.h"
#include "core/log.h"


Label::Label(std::string name, Widget* parent) :Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());
    // 设置字体路径
    SetFontPath(Resources::GetFontPath());
}

Label::~Label(){
    if (m_font_style) {
        delete (lv_style_t*)m_font_style;
    }
}

void Label::Create(ScreenLayer layer) {
    m_object = lv_label_create(layer);
}

void Label::SetText(const std::string& text) {
    if (text == m_text) {
        return;
    }
    m_text = text;
    lv_label_set_text(m_object, m_text.c_str());
}

std::string Label::GetText() {
    return m_text;
}

void Label::SetTextColor(const Color& color) {
    lv_color_t lv_color;
    lv_color.red = color.m_r;
    lv_color.green = color.m_g;
    lv_color.blue = color.m_b;
    lv_obj_set_style_text_color(m_object, lv_color, LV_PART_MAIN);
}

void Label::SetAlign(Align align) {
    lv_obj_set_style_text_align(m_object, (lv_text_align_t)align, LV_PART_MAIN);
}

std::string Label::GetFontPath() {
    return m_font_path;
}

void Label::SetFontPath(const std::string& font_path) {
    if (font_path == m_font_path) {
        return;
    }
    m_font_path = font_path;

    // 刷新文本
    Update();
}

void Label::SetFontSize(uint32_t size) {
    if (size == m_size) {
        return;
    }
    m_size = size;

    // 刷新文本
    Update();
}

uint32_t Label::GetFontSize() {
    return m_size;
}

void Label::Update() {
    /*Create a font*/
    lv_font_t* font = lv_freetype_font_create(
        GetFontPath().c_str(),
        LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
        GetFontSize(),
        LV_FREETYPE_FONT_STYLE_NORMAL);

    if (!font) {
        LOGW("freetype font create failed.");
        LOGW("font path:%s", GetFontPath().c_str());
        return;
    }

    /*Create style with the new font*/
    if (!m_font_style) {
        m_font_style = new lv_style_t;
    }
    lv_style_init((lv_style_t*)m_font_style);
    lv_style_set_text_font((lv_style_t*)m_font_style, font);
    lv_obj_add_style(m_object, (lv_style_t*)m_font_style, LV_PART_MAIN);
}
