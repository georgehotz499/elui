#include "object.h"
#include "core/log.h"

extern "C" {
#include "lvgl/lvgl.h"
#include "lvgl/src/misc/lv_timer_private.h"
}


Object::Object(std::string name, Object* parent) :m_name(name) {
    if (parent) {
        // 若指定父对象，调用 setParent 建立关系
        SetParent(parent);
    }
}

Object::~Object() {

}

std::string Object::GetName() {
    return m_name;
}

void Object::SetParent(Object* parent) {
    // 父对象不变，直接返回
    if (m_parent == parent) return;

    // 步骤1：从旧父对象的子列表中移除自己
    RemoveFromParent();

    // 步骤2：设置新父对象，并添加到新父的子列表
    m_parent = parent;
    if (m_parent) {
        m_parent->m_children.push_back(this);  // 加入新父的子列表
    }
}

Object* Object::GetParent() {
    return m_parent;
}

void Object::RemoveFromParent() {
    if (m_parent) {
        m_parent->m_children.remove(this);
    }
    m_parent = nullptr;
}

void Object::MoveToTop() {
    if (nullptr == m_parent) return;
    // 移除控件
    m_parent->m_children.remove(this);
    // 控件加入链表尾
    m_parent->m_children.push_back(this);
}

void Object::MoveToBottom() {
    if (nullptr == m_parent) return;
    // 移除控件
    m_parent->m_children.remove(this);
    // 控件加入链表头
    m_parent->m_children.push_front(this);
}

// 定时器回调函数
static void TimerCallback(lv_timer_t* timer) {
    // 获取传递的用户数据
    TimerInfo* user_data = (TimerInfo*)(timer->user_data);
    if (!user_data) {
        return;
    }

    // 执行定时器回调
    user_data->m_object->ExecuteTimerCallback(user_data->m_timer.m_timer_id);
}

void Object::StartTimer(int timer_id, int interval) {
    // 条件不符，不打开定时器
    if (timer_id < 0 || interval < 0) {
        LOGW("Failed to start timer. timer id:%d interval:%d", timer_id, interval);
        return;
    }

    // 定时器已打开，不重复打开
    if (m_timers.end() != m_timers.find(timer_id)) {
        LOGW("Timer had started. timer id:%d interval:%d", timer_id, interval);
        return;
    }
    TimerInfo* timer_info = new TimerInfo(timer_id, interval, this);

    // 创建定时器
    lv_timer_t* timer = lv_timer_create(TimerCallback, interval, timer_info);
    // 保存定时器信息
    m_timers[timer_id] = timer;
}

void Object::StopTimer(int timer_id) {
    auto it = m_timers.find(timer_id);
    if (m_timers.end() != it) {
        // 删除用户数据
        delete ((lv_timer_t*)it->second)->user_data;
        ((lv_timer_t*)it->second)->user_data = nullptr;
        // 彻底删除定时器（释放资源）
        lv_timer_del((lv_timer_t*)it->second);
        // 移除定时器信息
        m_timers.erase(timer_id);
    }
    else {
        LOGW("Timer id:%d not alive!!", timer_id);
    }
}

void Object::StopAllTimer() {
    for (auto it : m_timers) {
        // 删除用户数据
        delete ((lv_timer_t*)it.second)->user_data;
        ((lv_timer_t*)it.second)->user_data = nullptr;
        // 彻底删除定时器（释放资源）
        lv_timer_del((lv_timer_t*)it.second);
    }
    // 释放数据
    m_timers.clear();
}

void Object::ExecuteTimerCallback(int timer_id) {
    LOGW("Page %s not implement ExecuteTimerCallback.", GetName().c_str());
}
