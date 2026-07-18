#pragma once
#include "surface/template_surface.h"


class TemplateLogic {
    friend TemplateSurface;
public:
    TemplateLogic(std::string name, Widget* parent, ScreenLayer layer);
    ~TemplateLogic();

private:
    /**
     * @brief 初始化UI
     */
    void InitUi();

    /**
     * @brief 定时器回调
     * @param timer_id 定时器id
     * @return true 关闭定时器；false 定时器继续执行
     */
    bool TimerCallback(int timer_id);

    /**
     * @brief 按键点击事件
     */
    void ButtonClicked(Button* btn);

    /**
     * @brief 列表子项数回调
     * @return 返回子项数量
     */
    int ListItemCount(void);

    /**
     * @brief 列表子项刷新回调
     * @param item 子项指针
     * @param index 子项索引
     */
    void ListItemUpdate(Button* item, int index);

    /**
     * @brief 列表子项点击回调
     * @param item 子项指针
     * @param index 子项索引
     */
    void ListItemClicked(Button* item, int index);

    /**
     * @brief 滑条滑动回调
     * @param slider 滑条指针
     * @param progress 滑条当前进度值
     */
    void SliderProgressNotify(Slider* slider, int progress);

    /**
     * @brief 弧形滑条滑动回调
     * @param arc 滑条指针
     * @param progress 滑条当前进度值
     */
    void ArcProgressNotify(class Arc* arc, int progress);

    /**
     * @brief 单行输入框文本变化回调
     * @param input 单行输入框指针
     * @param text 输入框文本
     */
    void LineinputTextChanged(class Lineinput* input, const std::string& text);

public:
    /**
     * @brief 隐藏页面
     */
    void Hide();

    /**
     * @brief 显示页面
     */
    void Show();

private:
    // 页面ui
    TemplateSurface* ui{ nullptr };
};
