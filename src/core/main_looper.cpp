#include "main_looper.h"

extern "C" {
#include "lvgl/lvgl.h"
}

namespace {
    /**
     * @brief 主线程执行
     * @param user_data 自定义数据
    */
    //static void ConsumeMainTask(void * user_data) {
    //    MAINLOOPER->ExecuteMainMessageQueue();
    //}

    // 定时器列表
    static Object::Timer timer_list[] = { {0, 10}/*用于驱动主线程定时器回调*/};
}

MainLooper::MainLooper() : Object("MainLooper")
{
    // 初始化定时器
    InitTimer();
}

MainLooper::~MainLooper()
{
}

MainLooper* MainLooper::GetInstance() {
    static MainLooper ins;
    return &ins;
}

void MainLooper::SetMainThreadId(std::thread::id id) {
    m_main_thread_id = id;
}

const std::thread::id MainLooper::GetMainThreadId() {
    return m_main_thread_id;
}

void MainLooper::AddMainMessageQueue(MainMessageQueue callback) {
    AutoLock _l(m_message_queue_mutex);
    m_message_queue.push(callback);
}

void MainLooper::InitTimer() {
    for (auto info : timer_list) {
        StartTimer(info.m_timer_id, info.m_interval);
    }
}

MainLooper::MainMessageQueue MainLooper::GetCallback() {
    AutoLock _l(m_message_queue_mutex);
    if (!m_message_queue.empty()) {
        MainMessageQueue fun = m_message_queue.front();
        m_message_queue.pop();
        return fun;
    }
    return nullptr;
}

void MainLooper::ExecuteMainMessageQueue() {
    MainMessageQueue fun{ nullptr };
    while (nullptr != (fun = GetCallback()))
    {
        fun();
    }
}

void MainLooper::ExecuteTimerCallback(int timer_id) {
    // 执行主线程回调
    ExecuteMainMessageQueue();
}
