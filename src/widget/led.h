#pragma once
#include "widget.h"


class Led : public Widget {
public:
    Led(std::string name, Widget* parent);
    ~Led();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
     * @brief 获取LED最小亮度
     */
    static int BrightnessMin();

    /**
     * @brief 获取LED最大亮度
     */
    static int BrightnessMax();

    /**
     * @brief 设置LED颜色
     */
    void SetColor(const Color& color);

    /**
     * @brief 获取LED颜色
     */
    Color GetColor();

    /**
     * @brief 设置LED亮度0~100
     */
    void SetBrightness(int brightness);

    /**
     * @brief 获取LED亮度0~100
     */
    int GetBrightness();

    /**
     * @brief 点亮LED
     */
    void On();

    /**
     * @brief 熄灭LED
     */
    void Off();

    /**
     * @brief 切换LED亮灭状态
     */
    void Toggle();

    /**
     * @brief 设置LED亮灭状态
     */
    void SetOn(bool on);

    /**
     * @brief LED是否点亮
     */
    bool IsOn();

    /**
     * @brief 设置不透明度0~100
     */
    void SetOpacity(int opacity);

    /**
     * @brief 获取不透明度
     */
    int GetOpacity();

    /**
     * @brief 设置光晕宽度
     */
    void SetShadowWidth(int width);

    /**
     * @brief 获取光晕宽度
     */
    int GetShadowWidth();

    /**
     * @brief 设置光晕扩散
     */
    void SetShadowSpread(int spread);

    /**
     * @brief 获取光晕扩散
     */
    int GetShadowSpread();

private:
    /**
     * @brief 应用样式
     */
    void ApplyStyle();

    /**
     * @brief 限制亮度范围
     */
    int ClampBrightness(int brightness);

private:
    // LED颜色
    Color m_color{ 0X00, 0XA8, 0X6B };
    // LED亮度0~100
    int m_brightness{ 100 };
    // 不透明度
    int m_opacity{ 100 };
    // 光晕宽度
    int m_shadow_width{ 20 };
    // 光晕扩散
    int m_shadow_spread{ 3 };
};
