#include "thread.h"

AutoLock::AutoLock(Mutex& mutex) {
    m_mutex = &mutex;
    if (m_mutex) {
        m_mutex->lock();
    }
}

AutoLock::~AutoLock() {
    if (m_mutex) {
        m_mutex->unlock();
    }
}

Thread::Thread() {
}

Thread::Thread(std::function<bool(void)> fun) :m_fun(fun) {
}

Thread::~Thread() {

}

void Thread::Run() {
    // 线程正在运行，不可重复启动
    if (IsThreadRunning()) {
        return;
    }
    // 重置请求退出状态
    ResetRequestStatus();
    // 设置线程状态运行中
    SetThreadStatus(true);

    // 启动线程
    std::thread(&Thread::ExecuteThreadLoop, this).detach();
}

void Thread::Sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::thread::id Thread::GetId() {
    return std::this_thread::get_id();
}

bool Thread::IsThreadRunning() {
    AutoLock _l(m_is_running_mutex);
    return m_is_running;
}

void Thread::RequestQuit() {
    AutoLock _l(m_is_request_quit_mutex);
    m_is_request_quit = true;
}

void Thread::RequestQuitAndWait() {
    {
        AutoLock _l(m_is_request_quit_mutex);
        m_is_request_quit = true;
    }
    while (IsThreadRunning()) {
        // 延时5毫秒，避免轮询过快
        Sleep(5);
    }
}

bool Thread::ThreadLoop() {
    return false;
}

bool Thread::IsRequestQuit() {
    AutoLock _l(m_is_request_quit_mutex);
    return m_is_request_quit;
}

void Thread::ExecuteThreadLoop() {
    bool ret = false;
    do {
        // 传参回调函数
        if (m_fun) {
            ret = m_fun();
            ret = ThreadLoop() || ret;
        }
        // 未传参回调函数
        else {
            ret = ThreadLoop();
        }
    } while (ret);

    // 设置线程状态运行完毕
    SetThreadStatus(false);
}

void Thread::SetThreadStatus(bool status) {
    AutoLock _l(m_is_running_mutex);
    m_is_running = status;
}

void Thread::ResetRequestStatus() {
    AutoLock _l(m_is_request_quit_mutex);
    m_is_request_quit = false;
}
