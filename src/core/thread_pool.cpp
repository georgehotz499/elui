#include "thread_pool.h"

ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    // 创建固定数量的工作线程
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                { // 加锁取任务
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this]() {
                        return this->stop || !this->tasks.empty();
                        });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                // 执行任务
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable())
            worker.join();
    }
}

// 线程池数量
#define THREAD_COUNT 1

ThreadPoolManager::ThreadPoolManager():ThreadPool(THREAD_COUNT) {

}

ThreadPoolManager* ThreadPoolManager::GetInstance() {
    static ThreadPoolManager ins;
    return &ins;
}
