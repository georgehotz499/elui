#include "led.h"

extern "C" {
#include "lvgl/lvgl.h"
}


namespace {
    lv_color_t ToLvColor(const Color& color) {
        return lv_color_make(color.m_r, color.m_g, color.m_b);
    }

    uint8_t ToLvOpacity(int opacity) {
        return static_cast<uint8_t>(opacity * LV_OPA_COVER / 100);
    }

    uint8_t ToLvBrightness(int brightness) {
        return static_cast<uint8_t>(brightness * 255 / 100);
    }
}

Led::Led(std::string name, Widget* parent) : Widget(name, parent) {
    Create(parent ? parent->GetLayer() : SCREEN_MGR->GetActiveLayer());
}

Led::~Led() {

}

void Led::Create(ScreenLayer layer) {
    m_object = lv_led_create(layer);
    lv_obj_remove_flag(m_object, LV_OBJ_FLAG_SCROLLABLE);

    ApplyStyle();
    SetColor(m_color);
    SetBrightness(m_brightness);
}

int Led::BrightnessMin() {
    return 0;
}

int Led::BrightnessMax() {
    return 100;
}

void Led::SetColor(const Color& color) {
    m_color = color;

    if (m_object) {
        lv_led_set_color(m_object, ToLvColor(m_color));
    }
}

Color Led::GetColor() {
    return m_color;
}

void Led::SetBrightness(int brightness) {
    m_brightness = ClampBrightness(brightness);

    if (m_object) {
        lv_led_set_brightness(m_object, ToLvBrightness(m_brightness));
    }
}

int Led::GetBrightness() {
    return m_brightness;
}

void Led::On() {
    SetBrightness(BrightnessMax());
}

void Led::Off() {
    SetBrightness(BrightnessMin());
}

void Led::Toggle() {
    SetOn(!IsOn());
}

void Led::SetOn(bool on) {
    if (on) {
        On();
    }
    else {
        Off();
    }
}

bool Led::IsOn() {
    return GetBrightness() > ((BrightnessMin() + BrightnessMax()) >> 1);
}

void Led::SetOpacity(int opacity) {
    if (opacity < 0 || 100 < opacity) {
        return;
    }

    m_opacity = opacity;
    ApplyStyle();
}

int Led::GetOpacity() {
    return m_opacity;
}

void Led::SetShadowWidth(int width) {
    if (width < 0) {
        return;
    }

    m_shadow_width = width;
    ApplyStyle();
}

int Led::GetShadowWidth() {
    return m_shadow_width;
}

void Led::SetShadowSpread(int spread) {
    if (spread < 0) {
        return;
    }

    m_shadow_spread = spread;
    ApplyStyle();
}

int Led::GetShadowSpread() {
    return m_shadow_spread;
}

void Led::ApplyStyle() {
    if (!m_object) {
        return;
    }

    lv_obj_set_style_bg_opa(m_object, ToLvOpacity(m_opacity), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(m_object, m_shadow_width, LV_PART_MAIN);
    lv_obj_set_style_shadow_spread(m_object, m_shadow_spread, LV_PART_MAIN);

    lv_obj_set_style_bg_color(m_object, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(m_object, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_shadow_color(m_object, lv_color_white(), LV_PART_MAIN);
}

int Led::ClampBrightness(int brightness) {
    if (brightness < BrightnessMin()) {
        return BrightnessMin();
    }
    if (BrightnessMax() < brightness) {
        return BrightnessMax();
    }

    return brightness;
}
