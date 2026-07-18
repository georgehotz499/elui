#pragma once
#include "widget.h"

#include "core/color.h"
#include "core/image_ins.h"

#include <functional>


class Arc : public Widget, protected TouchListener {
public:
    // 滑条滑动回调
    using ProgressCallback = std::function<void(class Arc*, int)>;

public:
    Arc(std::string name, Widget* parent);
    ~Arc();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
     * @brief 设置主背景颜色
     */
    void SetMainColor(const Color& color);

    /**
     * @brief 获取主背景颜色
     */
    Color GetMainColor();

    /**
     * @brief 设置进度条颜色
     */
    void SetIndColor(const Color& color);

    /**
     * @brief 获取进度条颜色
     */
    Color GetIndColor();

    /**
     * @brief 设置knob颜色
     */
    void SetKnobColor(const Color& color);

    /**
     * @brief 获取knob颜色
     */
    Color GetKnobColor();

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
     * @brief 设置起始角度
     */
    void SetStartAngle(int angle);

    /**
     * @brief 获取起始角度
     */
    int GetStartAngle();

    /**
     * @brief 设置可滑动角度
     */
    void SetRangeAngle(int angle);

    /**
     * @brief 获取可滑动角度
     */
    int GetRangeAngle();

    /**
     * @brief 设置最小值
     */
    void SetMin(int value);

    /**
    * @brief 获取最小值
    */
    int GetMin();

    /**
     * @brief 设置最大值
     */
    void SetMax(int value);

    /**
    * @brief 获取最大值
    */
    int GetMax();

    /**
     * @brief 设置当前进度
     */
    void SetProgress(int progress);

    /**
     * @brief 获取当前进度
     */
    int GetProgress();

    /**
     * @brief 设置不可触摸半径
     */
    void SetUntouchableRadius(int radius);

    /**
    * @brief 获取不可触摸半径
    */
    int GetUntouchableRadius();

    /**
     * @brief 设置弧形滑条内边距
     */
    void SetPadding(int padding);

    /**
     * @brief 获取弧形滑条内边距
     */
    int GetPadding();

    /**
     * @brief 设置背景图片
     */
    void SetImageMain(const std::string& path);

    /**
     * @brief 获取背景图片
     */
    ImageDes GetImageMain();

    /**
     * @brief 设置进度图片
     */
    void SetImageIndicator(const std::string& path);

    /**
     * @brief 获取进度图片
     */
    ImageDes GetImageIndicator();

    /**
     * @brief 设置knob图片
     */
    void SetImageKnob(const std::string& path);

    /**
     * @brief 获取knob图片
     */
    ImageDes GetImageKnob();

    /**
    * @brief 是否隐藏knob
    */
    void SetKnobVisible(bool is_visible);

    /**
    * @brief knob是否显示
    */
    bool IsKnobVisible();

    /**
     * @brief 注册滑条滑动回调
     */
    void AddProgressCallback(ProgressCallback callback);

private:
    /**
    * @brief 执行滑条回调
    */
    void ExecuteProgressCallback(class Arc* slider, int progress);

    /**
    * @brief 触摸回调
    * @param widget 控件指针
    * @param gesture 手势
    * @return true 拦截事件传递；false 事件继续传递
    */
    bool Touch(Widget* widget, const Gesture& gesture) override;

private:
    // 背景颜色
    Color m_color_main{ 0X1F, 0X29, 0X37 };
    // 进度条颜色
    Color m_color_indicator{ 0XE7, 0X48, 0X9D };
    // knob颜色
    Color m_color_knob{ 0X8B, 0X5C, 0XF6 };
    // 圆角半径（只对纯色背景有效果）
    int m_radius{ 0 };
    // 不透明度
    int m_opacity{ 100 };
    // 起始角度
    int m_start_angle{ 135 };
    // 可滑动角度
    int m_range_angle{ 270 };
    // 滑条最小值
    int m_min{ 0 };
    // 滑条最大值
    int m_max{ 100 };
    // 当前进度
    int m_progress{ 0 };
    // 是否按下触碰控件
    bool m_press_ctrl{ false };
    // 不可触摸半径(控件的1/2宽度-m_disable_radius=弧形滑条knob的直径)
    int m_untouchable_radius{0};
    // 弧形滑条的内边距(控件的1/2宽度-m_disable_radius-2*m_padding=弧形滑条的宽度)
    int m_padding{ 0 };
    // 背景图
    ImageDes m_image_main{ nullptr };
    // 进度指示图
    ImageDes m_image_indicator{ nullptr };
    // knob图
    ImageDes m_image_knob{ nullptr };
    // knob是否显示
    bool m_knob_visible{ true };
    // 滑条滑动回调
    ProgressCallback m_progress_callback{ nullptr };
};
