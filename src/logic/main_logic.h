#include "widget/object.h"

#include "core/screen.h"

#include "logic/template_logic.h"


class MainLogic : public Object {
public:
    MainLogic(const std::string& name, ScreenLayer layer);

private:
    /**
     * @brief 初始化UI
     */
    void InitUi();

    /**
     * @brief 初始化页面定时器时执行
     */
    void InitTimer();

    /**
     * @brief 定时器回调
     * @param timer_id 定时器id
     * @return true 关闭定时器；false 定时器继续执行
     */
    bool TimerCallback(int timer_id);

    /**
    * @brief 定时器回调函数
    */
    void ExecuteTimerCallback(int timer_id) override;

private:
    // 显示层级指针
    ScreenLayer m_layer{ nullptr };
    // template页面
    TemplateLogic* m_templatelogic{ nullptr };
};
