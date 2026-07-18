#pragma once
#include "widget.h"
#include "core/color.h"


class Label : public Widget {
public:
    using Align = enum {
        kAlignAuto,
        kAlignLeft,
        kAlignCenter,
        kAlignRight
    };

public:
    Label(std::string name, Widget* parent);
    ~Label();
private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;
public:
    /**
    * @brief 设置文本
    */
    void SetText(const std::string& text);

    /**
    * @brief 获取文本
    */
    std::string GetText();

    /**
     * @brief 设置文本颜色
     */
    void SetTextColor(const Color& color);

    /**
     * @brief 设置文本对齐
     */
    void SetAlign(Align align);

    /**
     * @brief 获取字体全路径
     */
    std::string GetFontPath();

    /**
     * @brief 设置字体全路径
     */
    void SetFontPath(const std::string& font_path);

    /**
     * @brief 设置字体大小
     */
    void SetFontSize(uint32_t size);

    /**
     * @brief 获取字体大小
     */
    uint32_t GetFontSize();

private:
    /**
     * @brief 刷新文本
     */
    void Update();

private:
    // 文本内容
    std::string m_text;
    // 字体全路径
    std::string m_font_path;
    // 字体大小
    uint32_t m_size{ 16 };
    // 字体风格
    void* m_font_style{ nullptr };
};

