#include "lineinput.h"

#include "core/log.h"
#include "core/resources.h"

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

    /**
     * @brief 输入框事件回调
     * @param event 事件信息
     */
    static void EventHandle(lv_event_t* event) {
        Lineinput* input = static_cast<Lineinput*>(lv_event_get_user_data(event));
        if (!input) {
            return;
        }

        lv_event_code_t code = lv_event_get_code(event);
        switch (code) {
        case LV_EVENT_INSERT:
            input->ExecuteInsertReplace();
            break;
        case LV_EVENT_VALUE_CHANGED:
            input->ExecuteTextChangedCallback();
            break;
        default:
            break;
        }
    }
}

Lineinput::Lineinput(std::string name, Widget* parent) : Widget(name, parent) {
    // 创建对象
    Create(parent ? parent->GetLayer() : SCREEN_MGR->GetActiveLayer());
    // 设置字体路径
    SetFontPath(Resources::GetFontPath());
}

Lineinput::~Lineinput() {
    if (m_font_style) {
        delete (lv_style_t*)m_font_style;
        m_font_style = nullptr;
    }

    if (m_font) {
        lv_freetype_font_delete((lv_font_t*)m_font);
        m_font = nullptr;
    }
}

void Lineinput::Create(ScreenLayer layer) {
    m_object = lv_textarea_create(layer);
    // 设置为单行输入
    lv_textarea_set_one_line(m_object, true);
    // 设置点击文本可定位光标
    lv_textarea_set_cursor_click_pos(m_object, true);
    // 禁用滚动条显示
    lv_obj_set_scrollbar_mode(m_object, LV_SCROLLBAR_MODE_OFF);
    // 注册插入事件
    lv_obj_add_event_cb(m_object, EventHandle, LV_EVENT_INSERT, this);
    // 注册文本变化事件
    lv_obj_add_event_cb(m_object, EventHandle, LV_EVENT_VALUE_CHANGED, this);
}

void Lineinput::SetText(const std::string& text) {
    lv_textarea_set_text(m_object, text.c_str());
}

std::string Lineinput::GetText() {
    const char* text = lv_textarea_get_text(m_object);
    return text ? text : "";
}

void Lineinput::AddText(const std::string& text) {
    lv_textarea_add_text(m_object, text.c_str());
}

void Lineinput::AddChar(uint32_t ch) {
    lv_textarea_add_char(m_object, ch);
}

void Lineinput::DeleteChar() {
    lv_textarea_delete_char(m_object);
}

void Lineinput::DeleteCharForward() {
    lv_textarea_delete_char_forward(m_object);
}

void Lineinput::Clear() {
    lv_textarea_set_text(m_object, "");
}

void Lineinput::SetPlaceholderText(const std::string& text) {
    m_placeholder_text = text;
    lv_textarea_set_placeholder_text(m_object, m_placeholder_text.c_str());
}

std::string Lineinput::GetPlaceholderText() {
    const char* text = lv_textarea_get_placeholder_text(m_object);
    return text ? text : "";
}

void Lineinput::SetCursorPos(int pos) {
    lv_textarea_set_cursor_pos(m_object, pos);
}

int Lineinput::GetCursorPos() {
    return static_cast<int>(lv_textarea_get_cursor_pos(m_object));
}

void Lineinput::CursorLeft() {
    lv_textarea_cursor_left(m_object);
}

void Lineinput::CursorRight() {
    lv_textarea_cursor_right(m_object);
}

void Lineinput::SetCursorClickPos(bool enable) {
    lv_textarea_set_cursor_click_pos(m_object, enable);
}

bool Lineinput::GetCursorClickPos() {
    return lv_textarea_get_cursor_click_pos(m_object);
}

void Lineinput::SetAcceptedChars(const std::string& chars) {
    m_accepted_chars = chars;
    lv_textarea_set_accepted_chars(m_object, m_accepted_chars.empty() ? nullptr : m_accepted_chars.c_str());
}

void Lineinput::ClearAcceptedChars() {
    m_accepted_chars.clear();
    lv_textarea_set_accepted_chars(m_object, nullptr);
}

std::string Lineinput::GetAcceptedChars() {
    const char* chars = lv_textarea_get_accepted_chars(m_object);
    return chars ? chars : "";
}

void Lineinput::SetMaxLength(uint32_t length) {
    lv_textarea_set_max_length(m_object, length);
}

uint32_t Lineinput::GetMaxLength() {
    return lv_textarea_get_max_length(m_object);
}

void Lineinput::SetInsertReplace(const std::string& text) {
    m_insert_replace = text;
    m_insert_replace_enable = true;
}

void Lineinput::ClearInsertReplace() {
    m_insert_replace.clear();
    m_insert_replace_enable = false;
}

std::string Lineinput::GetInsertReplace() {
    return m_insert_replace;
}

void Lineinput::SetPasswordMode(bool enable) {
    lv_textarea_set_password_mode(m_object, enable);
}

bool Lineinput::GetPasswordMode() {
    return lv_textarea_get_password_mode(m_object);
}

void Lineinput::SetPasswordBullet(const std::string& bullet) {
    m_password_bullet = bullet;
    lv_textarea_set_password_bullet(m_object, m_password_bullet.c_str());
}

std::string Lineinput::GetPasswordBullet() {
    const char* bullet = lv_textarea_get_password_bullet(m_object);
    return bullet ? bullet : "";
}

void Lineinput::SetPasswordShowTime(uint32_t time) {
    lv_textarea_set_password_show_time(m_object, time);
}

uint32_t Lineinput::GetPasswordShowTime() {
    return lv_textarea_get_password_show_time(m_object);
}

void Lineinput::SetTextColor(const Color& color) {
    lv_obj_set_style_text_color(m_object, ToLvColor(color), LV_PART_MAIN);
}

void Lineinput::SetPlaceholderColor(const Color& color) {
    lv_obj_set_style_text_color(m_object, ToLvColor(color), LV_PART_TEXTAREA_PLACEHOLDER);
}

void Lineinput::SetBorderColor(const Color& color) {
    lv_obj_set_style_border_color(m_object, ToLvColor(color), LV_PART_MAIN);
}

void Lineinput::SetBorderWidth(int width) {
    if (width < 0) {
        return;
    }

    lv_obj_set_style_border_width(m_object, width, LV_PART_MAIN);
}

std::string Lineinput::GetFontPath() {
    return m_font_path;
}

void Lineinput::SetFontPath(const std::string& font_path) {
    if (font_path == m_font_path) {
        return;
    }
    m_font_path = font_path;

    // 刷新字体
    UpdateFont();
}

void Lineinput::SetFontSize(uint32_t size) {
    if (size == m_size) {
        return;
    }
    m_size = size;

    // 刷新字体
    UpdateFont();
}

uint32_t Lineinput::GetFontSize() {
    return m_size;
}

void Lineinput::AddTextChangedCallback(TextChangedCallback callback) {
    m_text_changed_callback = callback;
}

void Lineinput::ExecuteTextChangedCallback() {
    if (m_text_changed_callback) {
        m_text_changed_callback(this, GetText());
    }
}

void Lineinput::ExecuteInsertReplace() {
    if (m_insert_replace_enable) {
        lv_textarea_set_insert_replace(m_object, m_insert_replace.c_str());
    }
}

void Lineinput::UpdateFont() {
    if (m_font_path.empty() || !m_object) {
        return;
    }

    // 创建字体
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

    lv_font_t* old_font = (lv_font_t*)m_font;
    m_font = font;

    // 创建字体风格
    if (!m_font_style) {
        m_font_style = new lv_style_t;
    }
    lv_style_init((lv_style_t*)m_font_style);
    lv_style_set_text_font((lv_style_t*)m_font_style, (lv_font_t*)m_font);
    lv_obj_add_style(m_object, (lv_style_t*)m_font_style, LV_PART_MAIN);
    lv_obj_add_style(m_object, (lv_style_t*)m_font_style, LV_PART_TEXTAREA_PLACEHOLDER);

    if (old_font) {
        lv_freetype_font_delete(old_font);
    }
}
