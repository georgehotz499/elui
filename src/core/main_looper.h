#pragma once
#include "thread.h"
#include "widget/object.h"

#include <queue>
#include <functional>


class MainLooper : public Object
{
public:
    using MainMessageQueue = std::function<void ()>;
public:
    MainLooper();
    ~MainLooper();

    /**
     * @brief 获取主线程循环对象实例
     */
    static MainLooper* GetInstance();

    /**
     * @brief 这只主线程id
     */
    void SetMainThreadId(std::thread::id id);

    /**
     * @brief 获取主线程id
     */
    const std::thread::id GetMainThreadId();

    /**
     * @brief 注册主线程回调函数
     */
    void AddMainMessageQueue(MainMessageQueue callback);

private:
    /**
    * @brief 启动定时器执行主线程任务
    */
    void InitTimer();

    /**
    * @brief 获取可执行的回调函数
    * @return nullptr 所有函数执行完毕
    */
    MainMessageQueue GetCallback();

    /**
     * @brief 执行主线程回调函数
     */
    void ExecuteMainMessageQueue();

    /**
     * @brief 执行定时器回调
     * @param timer_id 定时器id
     */
    void ExecuteTimerCallback(int timer_id) override;

private:
    // 消息队列存储对象
    std::queue<MainMessageQueue> m_message_queue;
    Mutex m_message_queue_mutex;
    // 主线程id
    std::thread::id m_main_thread_id;
};
#define MAINLOOPER MainLooper::GetInstance()
