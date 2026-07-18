#pragma once
#include <iostream>
#include <functional>
#include <thread>
#include <mutex>

class Mutex : public std::mutex {
};

class AutoLock {
private:
    Mutex* m_mutex{ nullptr };
public:
    /**
    * @brief 自动锁
    */
    explicit AutoLock(Mutex& mutex);
    ~AutoLock();
};

class Thread {
public:
    /**
    * @brief 构造函数
    */
    Thread();

    /**
    * @brief 带线程执行回调参数构造函数
    */
    Thread(std::function<bool(void)> fun);
    virtual ~Thread();

    /**
    * @brief 线程启动函数
    */
    void Run();

    /**
    * @brief 休眠等待函数（单位：ms）
    */
    static void Sleep(int ms);

    /**
    * @brief 获取线程id函数
    */
    static std::thread::id GetId();

    /**
    * @brief 是否线程正在执行函数
    * @return true 线程正在执行；false 线程未在执行
    */
    bool IsThreadRunning();

    /**
    * @brief 请求线程退出
    */
    void RequestQuit();

    /**
    * @brief 请求并等待线程退出
    */
    void RequestQuitAndWait();

protected:
    /**
    * @brief 线程循环函数
    * @return true:线程循环函数继续执行；false:线程循环函数停止执行
    */
    virtual bool ThreadLoop() = 0;

    /**
    * @brief 是否退出线程请求退出
    * @return true:请求退出线程；false:线程继续执行
    */
    bool IsRequestQuit();

private:
    /**
    * @brief 执行线程循环函数
    */
    void ExecuteThreadLoop();

    /**
    * @brief 设置线程执行状态
    * @param status true:线程正在执行；false:线程执行完成
    */
    void SetThreadStatus(bool status);

    /**
    * @brief 重置线程执行状态为不执行
    */
    void ResetRequestStatus();

private:
    //  线程执行函数指针
    std::function<bool(void)> m_fun{ nullptr };
    //  是否线程正在执行标志
    bool m_is_running{ false };
    Mutex m_is_running_mutex;
    //   是否线程请求退出标志
    bool m_is_request_quit{ false };
    Mutex m_is_request_quit_mutex;
};
