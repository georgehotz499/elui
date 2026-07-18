#include "main_logic.h"

#include "core/log.h"


#define DEBUG LOGD("DEBUG!!");

// 定时器列表
static const Object::Timer timer_list[] = {
    {-1, 10},
};


MainLogic::MainLogic(const std::string& name, ScreenLayer layer) :Object(name),m_layer(layer) {
    // 初始化UI
    InitUi();
}

void MainLogic::InitUi() {
    // 开启定时器
    InitTimer();

    // 构架页面
    m_templatelogic = new TemplateLogic("TemplateLogic", nullptr, m_layer);
}

void MainLogic::InitTimer() {
    for (auto info : timer_list) {
        StartTimer(info.m_timer_id, info.m_interval);
    }
}

bool MainLogic::TimerCallback(int timer_id) {
    switch (timer_id) {
    default:
        LOGD("Unsolved timer_id:%d", timer_id);
        break;
    }

    return false;
}

void MainLogic::ExecuteTimerCallback(int timer_id) {
    // 执行定时器回调函数
    if (TimerCallback(timer_id)) {
        // 返回true，关闭定时器
        StopTimer(timer_id);
    }
}

