#pragma once
#include <list>
#include <string>
#include <unordered_map>


class Object {
public:
    // 定时器(mqtt有个struct Timer，放在外面回报错重复定义)
    class Timer {
    public:
        Timer(int id, int interval) :m_timer_id(id), m_interval(interval) {}
    public:
        // 定时器id（页面内唯一的一个大于等于0的值）
        int m_timer_id{ -1 };
        // 定时器执行周期（单位:ms）
        int m_interval{ 0 };
    };

public:
    explicit Object(std::string name, Object* parent = nullptr);
    virtual ~Object();

    /**
     * @brief 获取对象名称
     */
    std::string GetName();

    /**
     * @brief 设置父对象
     */
    void SetParent(Object* parent);

    /**
    * @brief 获取父控件指针
    * @return 返回nullptr代表此控件为最顶层控件
    */
    Object* GetParent();

    /**
     * @brief 从父对象移除自己
     */
    void RemoveFromParent();

    /**
     * @brief 将控件移动到父控件容器的顶层
     */
    virtual void MoveToTop();

    /**
     * @brief 将控件移动到父控件容器的顶层
     */
    virtual void MoveToBottom();

    /**
     * @brief 启动定时器
     * @param timer_id 定时器id（此值大于等于0，页面内不可重复）
     * @param interval 定时周期（此值大于等于0,单位ms）
     */
    void StartTimer(int timer_id, int interval);

    /**
    * @brief 关闭定时器
    * @param timer_id 定时器id（此值大于等于0，页面内不可重复）
    */
    void StopTimer(int timer_id);

    /**
     * @brief 关闭所有定时器
     */
    void StopAllTimer();

    /**
     * @brief 执行定时器回调
     * @param timer_id 定时器id
     */
    virtual void ExecuteTimerCallback(int timer_id);

protected:
    // 对象名称（同一个页面内不允许重复）
    std::string m_name;
    // 父对象指针
    Object* m_parent{ nullptr };
    // 子对象列表
    std::list<Object*> m_children;
    // 定时器id
    std::unordered_map<int, void*> m_timers;
};

// 定时器信息
class TimerInfo {
public:
    TimerInfo(int id, int interval, Object* obj) :m_timer(id, interval), m_object(obj) {};
public:
    Object::Timer m_timer;
    Object* m_object{ nullptr };
};

