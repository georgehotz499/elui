#pragma once
#include "widget.h"
#include "label.h"

#include "core/color.h"
#include "core/image_ins.h"

#include <functional>


class Button : public Widget {
public:
    // 按键点击回调
    using ClickedCallback = std::function<void(Button*)>;

    // 按键状态
    enum Status {
        kNormal,
        kNormalPress,
        kClicked,
        kClickedPress,
        kDisable
    };

public:
    Button(std::string name, Widget* parent);
    ~Button();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
     * @brief 设置按键正常状态颜色
     */
    void SetColorNormal(const Color& color);

    /**
    * @brief 获取按键正常状态颜色
    */
    Color GetColorNormal();

    /**
    * @brief 设置按键正常状态按下颜色
    */
    void SetColorNormalPress(const Color& color);

    /**
    * @brief 获取按键正常状态按下颜色
    */
    Color GetColorNormalPress();

    /**
     * @brief 设置按键按下状态颜色
     */
    void SetColorClicked(const Color& color);

    /**
    * @brief 获取按键按下状态颜色
    */
    Color GetColorClicked();

    /**
     * @brief 设置按键按下按压颜色
     */
    void SetColorClickedPress(const Color& color);

    /**
    * @brief 获取按键按下按压颜色
    */
    Color GetColorClickedPress();

    /**
     * @brief 设置按键失能状态颜色
     */
    void SetColorDisable(const Color& color);

    /**
    * @brief 获取按键失能状态颜色
    */
    Color GetColorDisable();

    /**
    * @brief 设置按键正常状态
    */
    void SetStatusNormal();

    /**
    * @brief 设置按键按下状态
    */
    void SetStatusPress();

    /**
    * @brief 设置按键松开状态
    */
    void SetStatusRelease();

    /**
    * @brief 设置按键点击状态
    */
    void SetStatusClicked();

    /**
    * @brief 设置按键失能状态
    */
    void SetStatusDisnable();

    /**
    * @brief 设置按键使能状态
    */
    void SetStatusEnable();

    /**
    * @brief 获取按键状态
    */
    Status GetStatus();

    /**
     * @brief 设置圆角（只对纯色背景有效果）
     */
    void SetRadius(int radius);

    /**
     * @brief 获取圆角半径（只对纯色背景有效果）
     */
    int GetRadius();

    /**
     * @brief 设置不透明度0~100
     */
    void SetOpacity(int opacity);

    /**
     * @brief 获取不透明度
     */
    int GetOpacity();

    /**
    * @brief 设置正常状态图片
    */
    void SetImageNormal(const std::string& path);

    /**
    * @brief 获取正常状态图片
    */
    ImageDes GetImageNormal();

    /**
    * @brief 设置正常状态按压图片
    */
    void SetImageNormalPress(const std::string& path);

    /**
    * @brief 获取正常状态按压图片
    */
    ImageDes GetImageNormalPress();

    /**
    * @brief 设Clicked状态图片
    */
    void SetImageClicked(const std::string& path);

    /**
    * @brief 获取Clicked状态图片
    */
    ImageDes GetImageClicked();

    /**
    * @brief 设置Clicked状态按压图片
    */
    void SetImageClickedPress(const std::string& path);

    /**
    * @brief 获取Clicked状态按压图片
    */
    ImageDes GetImageClickedPress();

    /**
    * @brief 设置失能状态图片
    */
    void SetImageDisable(const std::string& path);

    /**
    * @brief 获取失能状态图片
    */
    ImageDes GetImageDisable();

    /**
    * @brief 设置文本
    */
    void SetText(const std::string& text);

    /**
     * @brief 设置文本颜色
     */
    void SetTextColor(const Color& color);

    /**
     * @brief 设置文本对齐
     */
    void SetAlign(Label::Align align);

    /**
     * @brief 设置字体全路径
     */
    void SetFontPath(const std::string& font_path);

    /**
     * @brief 设置字体大小
     */
    void SetFontSize(uint32_t size);

    /**
     * @brief 注册按键点击回调
     */
    void AddClickedCallback(ClickedCallback callback);

    /**
     * @brief 执行按键点击回调
     */
    void ExecuteClickedCallback();

private:
    // 正常状态颜色
    Color m_color_normal{0XAB, 0XEF, 0X0E};
    // 正常状态按下颜色
    Color m_color_normal_press{0XFF, 0X00, 0XDB};
    // clicked状态颜色
    Color m_color_clicked{0XFF, 0X00, 0XDB};
    // clicked状态按下颜色
    Color m_color_clicked_press{0XAB, 0XEF, 0X0E};
    // 失能状态颜色
    Color m_color_disable{0XCC, 0XCC, 0XCC};
    // 按键状态
    Status m_status{ kNormal };
    // 圆角半径（只对纯色背景有效果）
    int m_radius{ 10 };
    // 不透明度
    int m_opacity{ 100 };
    // 正常状态图片
    ImageDes m_image_normal{ nullptr };
    // 正常状态按下图片
    ImageDes m_image_normal_press{ nullptr };
    // clicked状态图片
    ImageDes m_image_clicked{ nullptr };
    // clicked状态按下图片
    ImageDes m_image_clicked_press{ nullptr };
    // 失能状态图片
    ImageDes m_image_disable{ nullptr };
    // 文本控件
    Label* m_label{ nullptr };
    // 按键点击回调
    ClickedCallback m_clicked{ nullptr };
};
