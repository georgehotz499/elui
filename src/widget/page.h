#pragma once
#include "widget.h"
#include "core/screen.h"

#include <functional>


class Page : public Widget {
public:
    /**
     * @brief 定时器回调
     * @param timer_id 定时器id
     * @return true 关闭定时器；false 定时器继续执行
     */
    using TimeoutCallback = std::function<bool(int timer_id)>;

public:
    Page(std::string name, Widget* parent, ScreenLayer layer = SCREEN_MGR->GetActiveLayer());
    ~Page();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
     * @brief 获取页面所在层级
     */
    int GetLevel();

    /**
     * @brief 将控件移动到父控件容器的顶层
     */
    virtual void MoveToTop();

    /**
     * @brief 将控件移动到父控件容器的顶层
     */
    virtual void MoveToBottom();

    /**
     * @brief 移动页面所在图层
     * @param page 页面指针
     * @param level 移动到的图层(取值：0~4)
     */
    void MoveLayer(int level);

    /**
    * @brief 注册定时器回调
    */
    void AddTimeoutCallback(TimeoutCallback callback);

private:
    /**
    * @brief 定时器回调函数
    */
    void ExecuteTimerCallback(int timer_id) override;

private:
    TimeoutCallback m_timeout_fun{ nullptr };
    // 页面所在层级
    int m_layer{ 0 };
};

