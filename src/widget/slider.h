#pragma once
#include "widget.h"

#include "core/color.h"
#include "core/image_ins.h"

#include <functional>


class Slider : public Widget, protected TouchListener {
public:
    /**
     * @brief 滑条方向 
     */
    enum Orientation {
        kHor, // 横向滑动
        kVer  // 纵向滑动
    };

    // 滑条滑动回调
    using ProgressCallback = std::function<void(Slider*,int)>;

public:
    Slider(std::string name, Widget* parent);
    ~Slider();

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
     * @brief 设置滑动方向
     */
    void SetOrientation(Orientation orientation);

    /**
     * @brief 获取滑动方向
     */
    Orientation GetOrientation();

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
     * @brief 设置最小值
     */
    void SetMinValue(int min);

    /**
    * @brief 获取最小值
    */
    int GetMinValue();

    /**
     * @brief 设置最大值
     */
    void SetMaxValue(int max);

    /**
     * @brief 获取最大值
     */
    int GetMaxValue();

    /**
    * @brief 设置最大最小值
    * @param min 最小值
    * @param max 最大值
    */
    void SetRange(int min, int max);

    /**
     * @brief 设置当前值
     */
    void SetProgress(int progress);

    /**
     * @brief 获取当前值
     */
    int GetProgress();

    /**
     * @brief 设置内边距
     */
    void SetPadding(int paddding);

    /**
     * @brief 获取内边距
     */
    int GetPadding();

    /**
    * @brief 设置控件位置大小
    */
    void SetGeometry(int x, int y, int w, int h);

    /**
    * @brief 获取控件大小
    */
    Rect GetGeometry();

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
    * @brief 获取进度指示图实际进度的存储对象
    */
    ImageDes GetImageIndicatorReal();

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
    void ExecuteProgressCallback(Slider* slider, int progress);

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
    // 控件滑动方向（默认水平滑动）
    Orientation m_orientation{ kHor };
    // 圆角半径（只对纯色背景有效果）
    int m_radius{ 0 };
    // 不透明度
    int m_opacity{ 100 };
    // 最小值
    int m_min{ 0 };
    // 最大值
    int m_max{ 100 };
    // 当前进度值
    int m_progress{ 0 };
    // 是否按下触碰控件
    bool m_press_ctrl{ false };
    // 内边距
    int m_padding{ 0 };
    // 滑条矩形区域
    Rect m_geometry;
    // 背景图
    ImageDes m_image_main{ nullptr };
    // 进度指示图
    ImageDes m_image_indicator{ nullptr };
    // 进度指示图实际进度的存储对象
    ImageDes m_image_indicator_real{ nullptr };
    // knob图
    ImageDes m_image_knob{ nullptr };
    // knob是否显示
    bool m_knob_visible{ true };
    // 滑条滑动回调
    ProgressCallback m_progress_callback{ nullptr };
};
