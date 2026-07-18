#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>


class ThreadPool {
public:
    /**
     * @brief 线程池构造
     * @param num_threads 线程池数
     */
    explicit ThreadPool(size_t num_threads);

    // 线程池销毁，等待线程退出
    virtual ~ThreadPool();

    // 提交任务
    template<class F, class... Args>
    void Enqueue(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {   // 线程安全加入任务队列
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(task);
        }
        condition.notify_one(); // 唤醒一个线程
    }

private:
    std::vector<std::thread> workers;           // 工作线程集合
    std::queue<std::function<void()>> tasks;    // 任务队列
    std::mutex queue_mutex;                     // 队列互斥锁
    std::condition_variable condition;          // 条件变量控制线程唤醒
    std::atomic<bool> stop;
};

class ThreadPoolManager : public ThreadPool {
public:
    ThreadPoolManager();
    static ThreadPoolManager* GetInstance();
};
#define THREADPOOL_MGR ThreadPoolManager::GetInstance()
