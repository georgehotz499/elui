#pragma once
#include "thread.h"

#include <map>
#include <string>
#include <functional>

class Notify {
public:
    // 回调参数类型
    class NotifyArg {
    public:
        // 添加虚析构函数，使类成为多态类型
        virtual ~NotifyArg() = default;
    };

    // 不带参数回调函数类型
    using WithoutArcCallback = std::function<void ()>;
    // 带参数回调函数类型
    using WithArcCallback = std::function<void (NotifyArg*)>;

public:
    /**
    * @brief 添加不带参数消息通知函数
    * @param key 回调参数key值
    * @param fun 回调函数
    */
    void AddCallback(const std::string& key, WithoutArcCallback fun);

    /**
    * @brief 执行不带参数消息通知函数
    * @param key 回调参数key值
    */
    void ExecuteCallback(const std::string& key);

    /**
    * @brief 添加带参数消息通知函数
    * @param key 回调参数key值
    * @param fun 回调函数
    */
    void AddCallback(const std::string& key, WithArcCallback fun);

    /**
    * @brief 移除消息通知函数
    * @param key 回调参数key值
    * @param fun 回调函数
    */
    void RemoveCallback(const std::string& key);

    /**
    * @brief 执行带参数消息通知函数
    * @param key 回调参数key值
    * @param argc 数据指针
    */
    void ExecuteCallback(const std::string& key, NotifyArg* argc);

    /**
     * @brief 检查关键词是否注册回调
     * @param key 关键词
     * @return true 已注册回调；false 未注册回调
     */
    bool CheckKeyExist(const std::string& key);

private:
    // 回调函数不带参数map
    std::map<std::string, WithoutArcCallback> m_without_argc;
    Mutex m_without_argc_mutex;

    // 回调函数带参数map
    std::map<std::string, WithArcCallback> m_with_argc;
    Mutex m_with_argc_mutex;
};

class NotifyManager : public Notify {
public:
    static NotifyManager* GetInstance();
};

#define NOTIFY_MGR NotifyManager::GetInstance()
