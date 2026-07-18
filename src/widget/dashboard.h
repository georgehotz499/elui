#pragma once
#include "widget.h"

#include "core/image_ins.h"


class Dashboard : public Widget {
public:
    Dashboard(std::string name, Widget* parent);
    ~Dashboard();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
     * @brief 设置背景图片
     */
    void SetImageMain(const std::string& path);

    /**
     * @brief 获取背景图片
     */
    ImageDes GetImageMain();

    /**
     * @brief 设置前景图片
     */
    void SetImageIndicator(const std::string& path);

    /**
     * @brief 获取前景图片
     */
    ImageDes GetImageIndicator();

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
     * @brief 设置当前值
     */
    void SetValue(int value);

    /**
     * @brief 获取当前值
     */
    int GetValue();

    /**
     * @brief 设置指针图片
     */
    void SetImagePointer(const std::string& path);

    /**
     * @brief 获取指针图片
     */
    ImageDes GetImagePointer();

    /**
     * @brief 设置指针图片原始方向角度（默认270°，即图片默认指向上方）
     */
    void SetImagePointerBaseAngle(int angle);

    /**
     * @brief 获取指针图片原始方向角度
     */
    int GetImagePointerBaseAngle();

private:
    /**
    * @brief 限制当前值在最大值和最小值之间
    */
    void ClampValue();

private:
    // 背景图
    ImageDes m_image_main{ nullptr };
    // 前景图
    ImageDes m_image_indicator{ nullptr };
    // 不透明度
    int m_opacity{ 100 };
    // 起始角度
    int m_start_angle{ 135 };
    // 可滑动角度
    int m_range_angle{ 270 };
    // 最小值
    int m_min{ 0 };
    // 最大值
    int m_max{ 100 };
    // 当前值
    int m_value{ 0 };
    // 指针图片
    ImageDes m_image_pointer{ nullptr };
    // 指针图片原始方向角度
    int m_image_pointer_base_angle{ 270 };
};
