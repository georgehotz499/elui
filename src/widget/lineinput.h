#pragma once
#include "widget.h"

#include <functional>
#include <string>


class Lineinput : public Widget {
public:
    using TextChangedCallback = std::function<void(Lineinput*, const std::string&)>;

public:
    Lineinput(std::string name, Widget* parent);
    ~Lineinput();

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
     * @brief 追加文本
     */
    void AddText(const std::string& text);

    /**
     * @brief 追加字符
     */
    void AddChar(uint32_t ch);

    /**
     * @brief 删除光标左侧字符
     */
    void DeleteChar();

    /**
     * @brief 删除光标右侧字符
     */
    void DeleteCharForward();

    /**
     * @brief 清空文本
     */
    void Clear();

    /**
     * @brief 设置占位文本
     */
    void SetPlaceholderText(const std::string& text);

    /**
     * @brief 获取占位文本
     */
    std::string GetPlaceholderText();

    /**
     * @brief 设置光标位置
     */
    void SetCursorPos(int pos);

    /**
     * @brief 获取光标位置
     */
    int GetCursorPos();

    /**
     * @brief 光标左移
     */
    void CursorLeft();

    /**
     * @brief 光标右移
     */
    void CursorRight();

    /**
     * @brief 设置点击文本定位光标
     */
    void SetCursorClickPos(bool enable);

    /**
     * @brief 获取点击文本定位光标状态
     */
    bool GetCursorClickPos();

    /**
     * @brief 设置可输入字符
     */
    void SetAcceptedChars(const std::string& chars);

    /**
     * @brief 清除可输入字符限制
     */
    void ClearAcceptedChars();

    /**
     * @brief 获取可输入字符
     */
    std::string GetAcceptedChars();

    /**
     * @brief 设置最大输入长度
     */
    void SetMaxLength(uint32_t length);

    /**
     * @brief 获取最大输入长度
     */
    uint32_t GetMaxLength();

    /**
     * @brief 设置插入替换文本
     */
    void SetInsertReplace(const std::string& text);

    /**
     * @brief 清除插入替换文本
     */
    void ClearInsertReplace();

    /**
     * @brief 获取插入替换文本
     */
    std::string GetInsertReplace();

    /**
     * @brief 设置密码模式
     */
    void SetPasswordMode(bool enable);

    /**
     * @brief 获取密码模式
     */
    bool GetPasswordMode();

    /**
     * @brief 设置密码替换字符
     */
    void SetPasswordBullet(const std::string& bullet);

    /**
     * @brief 获取密码替换字符
     */
    std::string GetPasswordBullet();

    /**
     * @brief 设置密码显示时间
     */
    void SetPasswordShowTime(uint32_t time);

    /**
     * @brief 获取密码显示时间
     */
    uint32_t GetPasswordShowTime();

    /**
     * @brief 设置文本颜色
     */
    void SetTextColor(const Color& color);

    /**
     * @brief 设置占位文本颜色
     */
    void SetPlaceholderColor(const Color& color);

    /**
     * @brief 设置边框颜色
     */
    void SetBorderColor(const Color& color);

    /**
     * @brief 设置边框宽度
     */
    void SetBorderWidth(int width);

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

    /**
     * @brief 注册文本变化回调
     */
    void AddTextChangedCallback(TextChangedCallback callback);

    /**
     * @brief 执行文本变化回调
     */
    void ExecuteTextChangedCallback();

    /**
     * @brief 执行插入替换
     */
    void ExecuteInsertReplace();

private:
    /**
     * @brief 刷新字体
     */
    void UpdateFont();

private:
    // 字体全路径
    std::string m_font_path;
    // 字体大小
    uint32_t m_size{ 16 };
    // 字体对象
    void* m_font{ nullptr };
    // 字体风格
    void* m_font_style{ nullptr };
    // 占位文本
    std::string m_placeholder_text;
    // 可输入字符
    std::string m_accepted_chars;
    // 插入替换文本
    std::string m_insert_replace;
    // 是否启用插入替换
    bool m_insert_replace_enable{ false };
    // 密码替换字符
    std::string m_password_bullet{ "*" };
    // 文本变化回调
    TextChangedCallback m_text_changed_callback{ nullptr };
};
