#pragma once
#include "json_manager.h"

class Message {
public:
    // 消息类型
    enum MsgType {
        kTypeNone = 0,  // 未定义类型
        kTypeStart      // 视频播放
    };

public:
    static Message Parse(const std::string& msg);

    /**
    * @brief 设置what参数
    */
    void SetWhat(const MsgType& what);

    /**
    * @brief 设置msg参数
    */
    void SetMsg(const Json& msg);

    /**
     * @brief 将message对象转换为json字符串
     */
    std::string ToJsonString();

public:
    // 消息类型
    MsgType m_what{ kTypeNone };
    // 消息数据信息
    Json m_msg;
};
