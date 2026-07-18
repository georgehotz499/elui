#include "notify.h"
#include "log.h"


void Notify::AddCallback(const std::string& key, WithoutArcCallback fun) {
    AutoLock _l(m_without_argc_mutex);
    m_without_argc[key] = fun;
}

void Notify::ExecuteCallback(const std::string& key) {
    AutoLock _l(m_without_argc_mutex);
    auto it = m_without_argc.find(key);
    if (m_without_argc.end() != it) {
        it->second();
        return;
    }
    LOGW("Not found key %s", key.c_str());
}

void Notify::AddCallback(const std::string& key, WithArcCallback fun) {
    AutoLock _l(m_with_argc_mutex);
    m_with_argc[key] = fun;
}

void Notify::RemoveCallback(const std::string& key) {
    // 移除不带参数的回调
    {
        AutoLock _l(m_without_argc_mutex);
        m_without_argc.erase(key);
    }
    // 移除带参数的回调
    {
        AutoLock _l(m_with_argc_mutex);
        m_with_argc.erase(key);
    }
}

void Notify::ExecuteCallback(const std::string& key, NotifyArg* argc) {
    AutoLock _l(m_with_argc_mutex);
    auto it = m_with_argc.find(key);
    if (m_with_argc.end() != it) {
        it->second(argc);
        return;
    }
    LOGW("Not found key %s", key.c_str());
}

bool Notify::CheckKeyExist(const std::string& key) {
    // 检查不带参数的回调
    {
        AutoLock _l(m_without_argc_mutex);
        auto it = m_without_argc.find(key);
        if (m_without_argc.end() != it) {
            return true;
        }
    }
    // 检查带参数的回调
    {
        AutoLock _l(m_with_argc_mutex);
        auto it = m_with_argc.find(key);
        if (m_with_argc.end() != it) {
            return true;
        }
    }
    return false;
}

NotifyManager* NotifyManager::GetInstance() {
    static NotifyManager ins;
    return &ins;
}
