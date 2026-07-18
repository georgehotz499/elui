#include "button.h"

#include "core/log.h"

extern "C" {
#include "lvgl/lvgl.h"
}


/**
 * @brief 绘制按键背景颜色
 * @param layer 控件图层
 * @param btn 按键指针
 */
static void DrawColor(lv_layer_t* layer, Button* btn) {
    // 初始化矩形绘制描述符
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);

    Button::Status status = btn->GetStatus();
    Color color =   Button::kNormal == status ? btn->GetColorNormal() :
                    Button::kNormalPress == status ? btn->GetColorNormalPress() :
                    Button::kClicked == status ? btn->GetColorClicked() :
                    Button::kClickedPress == status ? btn->GetColorClickedPress() : btn->GetColorDisable();

    rect_dsc.bg_color.red = color.m_r;
    rect_dsc.bg_color.green = color.m_g;
    rect_dsc.bg_color.blue = color.m_b;
    rect_dsc.bg_opa = static_cast<uint8_t>(btn->GetOpacity() * 255 / 100);
    rect_dsc.radius = btn->GetRadius();

    // 定义矩形区域
    Rect rect = btn->GetGlobalGeometry();
    lv_area_t area = { rect.m_x, rect.m_y, rect.m_x + rect.m_width - 1, rect.m_y + rect.m_height - 1 };

    // 绘制矩形
    lv_draw_rect(layer, &rect_dsc, &area);
}

/**
 * @brief 绘制按键背景图片
 * @param layer 控件图层
 * @param btn 按键指针
 */
static void DrawImage(lv_layer_t* layer, Button* btn) {
    Button::Status status = btn->GetStatus();
    lv_image_dsc_t* img_dsc = (lv_image_dsc_t*)(Button::kNormal == status ? btn->GetImageNormal() :
                                    Button::kNormalPress == status ? btn->GetImageNormalPress() :
                                    Button::kClicked == status ? btn->GetImageClicked() :
                                    Button::kClickedPress == status ? btn->GetImageClickedPress() : btn->GetImageDisable());

    lv_draw_image_dsc_t draw_image;
    lv_draw_image_dsc_init(&draw_image);
    draw_image.src = img_dsc;
    draw_image.opa = static_cast<uint8_t>(btn->GetOpacity() * 255 / 100);

    // 定义矩形区域
    Rect rect = btn->GetGlobalGeometry();
    lv_area_t area = { rect.m_x, rect.m_y, rect.m_x + rect.m_width - 1, rect.m_y + rect.m_height - 1 };

    // 绘制图片
    lv_draw_image(layer, &draw_image, &area);
}

/**
 * @brief 处理绘制逻辑
 * @param layer 控件图层
 * @param btn 按键指针
 */
static void DrawMain(lv_layer_t* layer, Button* btn) {
    switch (btn->GetStatus()) {
    case Button::kNormal:
        if (btn->GetImageNormal()) {
            DrawImage(layer, btn);
            return;
        }
        break;
    case Button::kNormalPress:
        if (btn->GetImageNormalPress()) {
            DrawImage(layer, btn);
            return;
        }
        break;
    case Button::kClicked:
        if (btn->GetImageClicked()) {
            DrawImage(layer, btn);
            return;
        }
        break;
    case Button::kClickedPress:
        if (btn->GetImageClickedPress()) {
            DrawImage(layer, btn);
            return;
        }
        break;
    case Button::kDisable:
        if (btn->GetImageDisable()) {
            DrawImage(layer, btn);
            return;
        }
        break;
    default:
        LOGW("unsolved status %d", static_cast<int>(btn->GetStatus()));
        break;
    }

    // 绘制纯色
    DrawColor(layer, btn);
}

/**
 * @brief 按键事件处理
 * @param event 事件信息
 */
static void EventHandler(lv_event_t* event) {
    // 获取事件码
    lv_event_code_t code = lv_event_get_code(event);
    Button* btn = static_cast<Button*>(lv_event_get_user_data(event));

    //LOGI("button %s event code：%d", btn->GetName().c_str(), static_cast<int>(code));

    switch (code) {
    case LV_EVENT_PRESSED:
        // 失能状态，直接刷新然后break
        if (Button::kDisable == btn->GetStatus()) {
            btn->Refresh();
            break;
        }
        // 设置按压状态
        btn->SetStatusPress();
        break;
    case LV_EVENT_RELEASED:
        // 失能状态，直接刷新然后break
        if (Button::kDisable == btn->GetStatus()) {
            btn->Refresh();
            break;
        }
        // 当前正常状态按压
        else if (Button::kNormalPress == btn->GetStatus()) {
            btn->SetStatusClicked();
        }
        // 当前clicked状态按压
        else if (Button::kClickedPress == btn->GetStatus()) {
            btn->SetStatusNormal();
        }
        break;
    case LV_EVENT_DRAW_MAIN:
    {
        // 获取图层指针
        lv_layer_t* layer = lv_event_get_layer(event);
        DrawMain(layer, btn);
    }
        break;
    case LV_EVENT_CLICKED:
        // 失能状态，不执行点击回调
        if (Button::kDisable == btn->GetStatus()) {
            break;
        }
        btn->ExecuteClickedCallback();
        break;
    }
}

// 定义要监听的事件列表
static lv_event_code_t event_list[] = {
    // 按下
    LV_EVENT_PRESSED, // 1
    // 松开
    LV_EVENT_RELEASED, // 8
    // 绘制主体
    LV_EVENT_DRAW_MAIN, // 26
    // 单击
    LV_EVENT_CLICKED, // 7
};

Button::Button(std::string name, Widget* parent) :Widget(name, parent) {
    // 创建对象
    Create(parent->GetLayer());

    // 创建文本控件
    m_label = new Label(name + "Label", this);
    // 设置文本居中对齐
    m_label->SetAlign(Label::kAlignCenter);
    // 控件在父控件居中
    lv_obj_center(m_label->GetLayer());

    // 绑定点击事件
    for (int i = 0; i < sizeof(event_list) / sizeof(event_list[0]); i++) {
        lv_obj_add_event_cb(m_object, EventHandler, event_list[i], this);
    }
}

Button::~Button() {
    // 析构图片数据
    ImageIns::DestroyImageDes(m_image_normal);
    ImageIns::DestroyImageDes(m_image_normal_press);
    ImageIns::DestroyImageDes(m_image_clicked);
    ImageIns::DestroyImageDes(m_image_clicked_press);
    ImageIns::DestroyImageDes(m_image_disable);
}

void Button::Create(ScreenLayer layer) {
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

void Button::SetColorNormal(const Color& color) {
    m_color_normal = color;
}

Color Button::GetColorNormal() {
    return m_color_normal;
}

void Button::SetColorNormalPress(const Color& color) {
    m_color_normal_press = color;
}

Color Button::GetColorNormalPress() {
    return m_color_normal_press;
}

void Button::SetColorClicked(const Color& color) {
    m_color_clicked = color;
}

Color Button::GetColorClicked() {
    return m_color_clicked;
}

void Button::SetColorClickedPress(const Color& color) {
    m_color_clicked_press = color;
}

Color Button::GetColorClickedPress() {
    return m_color_clicked_press;
}

void Button::SetColorDisable(const Color& color) {
    m_color_disable = color;
}

Color Button::GetColorDisable() {
    return m_color_disable;
}

void Button::SetStatusNormal() {
    if (kNormal == m_status) {
        return;
    }
    m_status = kNormal;

    Refresh();
}

void Button::SetStatusPress() {
    if (kNormal == m_status) {
        m_status = kNormalPress;
    }
    else if (kClicked == m_status) {
        m_status = kClickedPress;
    }
    else {
        return;
    }

    Refresh();
}

void Button::SetStatusRelease() {
    if (kNormalPress == m_status) {
        m_status = kNormal;
    }
    else if (kClickedPress == m_status) {
        m_status = kClicked;
    }
    else {
        return;
    }

    Refresh();
}

void Button::SetStatusClicked() {
    if (kClicked == m_status) {
        return;
    }
    m_status = kClicked;

    Refresh();
}

void Button::SetStatusDisnable() {
    if (kDisable == m_status) {
        return;
    }
    m_status = kDisable;

    Refresh();
}

void Button::SetStatusEnable() {
    if (kDisable != m_status) {
        return;
    }
    m_status = kNormal;

    Refresh();
}

Button::Status Button::GetStatus() {
    return m_status;
}

void Button::SetRadius(int radius) {
    if (0 > radius) {
        return;
    }
    m_radius = radius;
}

int Button::GetRadius() {
    return m_radius;
}

void Button::SetOpacity(int opacity) {
    if (0 > opacity || 100 < opacity) {
        return;
    }
    m_opacity = opacity;
}

int Button::GetOpacity() {
    return m_opacity;
}

void Button::SetImageNormal(const std::string& path) {
    ImageIns::DestroyImageDes(m_image_normal); 
    m_image_normal = ImageIns::ParseImage(path); 
}

ImageDes Button::GetImageNormal() {
    return m_image_normal;
}

void Button::SetImageNormalPress(const std::string& path) {
    ImageIns::DestroyImageDes(m_image_normal_press);
    m_image_normal_press = ImageIns::ParseImage(path);
}

ImageDes Button::GetImageNormalPress() {
    return m_image_normal_press;
}

void Button::SetImageClicked(const std::string& path) {
    ImageIns::DestroyImageDes(m_image_clicked);
    m_image_clicked = ImageIns::ParseImage(path);
}

ImageDes Button::GetImageClicked() {
    return m_image_clicked;
}

void Button::SetImageClickedPress(const std::string& path) {
    ImageIns::DestroyImageDes(m_image_clicked_press);
    m_image_clicked_press = ImageIns::ParseImage(path);
}

ImageDes Button::GetImageClickedPress() {
    return m_image_clicked_press;
}

void Button::SetImageDisable(const std::string& path) {
    ImageIns::DestroyImageDes(m_image_disable);
    m_image_disable = ImageIns::ParseImage(path);
}

ImageDes Button::GetImageDisable() {
    return m_image_disable;
}

void Button::SetText(const std::string& text) {
    m_label->SetText(text);
}

void Button::SetTextColor(const Color& color) {
    m_label->SetTextColor(color);
}

void Button::SetAlign(Label::Align align) {
    m_label->SetAlign(align);
}

void Button::SetFontPath(const std::string& font_path) {
    m_label->SetFontPath(font_path);
}

void Button::SetFontSize(uint32_t size) {
    m_label->SetFontSize(size);
}

void Button::AddClickedCallback(ClickedCallback callback) {
    m_clicked = callback;
}

void Button::ExecuteClickedCallback() {
    if (m_clicked) {
        m_clicked(this);
    }
}
