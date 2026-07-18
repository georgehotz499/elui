#pragma once
#include "thread.h"

#include <string>
#include <functional>
#include <vector>

#include "MQTTClient.h"


// MQTT消息
class MqttMsg {
public:
    // mqtt topic
    std::string m_topic;
    // mqtt message
    std::string m_message;
};

class Mqtt : protected Thread {
public:
    // MQTT数据接收回调
    using MsgReceive = std::function<void(const MqttMsg&)>;
    // MQTT初始化完成，连接服务器成功回调
    using ConnServer = std::function<void()>;

protected:
    virtual bool ThreadLoop() override;

public:
    // 设置服务器地址
    void SetMqttHost(const std::string& host);

    // 获取服务器地址
    std::string GetMqttHost();

    // 设置服务器端口
    void SetMqttPort(const std::string& port);

    // 获取服务器端口
    std::string GetMqttPort();

    // 设置用户名
    void SetMqttUserName(const std::string& user_name);

    // 获取用户名
    std::string GetMqttUserName();

    // 设置用户密码
    void SetMqttUserPassword(const std::string& password);

    // 获取用户密码
    std::string GetMqttUserPassword();

    // 设置客户端id
    void SetClientId(const std::string& client_id);

    // 设置订阅主题
    void SetTopic(const std::string& topic);

    void SetTopic(const std::vector<std::string>& topic);

    // 追加订阅主题
    void AppendTopic(const std::string& topic);

    // 连接MQTT服务器
    virtual void ConnectServer();

    /**
    * @brief 发布消息
    * @param topic 主题
    * @param message 发布的消息
    * @return true 发布成功；false 发布失败
    */
    bool Publish(const std::string& topic, const std::string& message);

    // 注册数据接收回调
    void AddMsgRecvCallback(MsgReceive callback);

    // 执行数据接收回调
    virtual void ExecuteMsgRecvCallback(const MqttMsg& msg);

    // 注册MQTT服务器连接成功回调
    void AddConnServerCallback(ConnServer callback);

    // 执行MQTT服务器连接成功回调
    virtual void ExecuteConnServerCallback();

    /**
     * @brief 设置连接状态
     * @param status true 已连接；false 未连接
     */
    void SetConnectStatus(bool status);

    /**
     * @brief 获取连接状态
     */
    bool GetConnectStatus();

private:
    // MQTT服务器地址
    std::string m_mqtt_host;
    // MQTT服务器端口
    std::string m_mqtt_port{ "1883" };
    // MQTT用户名
    std::string m_user_name;
    // MQTT用户密码
    std::string m_password;
    // MQTT客户端id
    std::string m_client_id;
    // MQTT订阅主题
    std::vector<std::string> m_topic_vec;

    // 当前Mqtt是否断开连接
    bool m_connect_status{ false };

    // 数据接收回调
    MsgReceive m_recv_callback{ nullptr };
    // MQTT服务器连接成功回调
    ConnServer m_conn_server_callback{ nullptr };
    // MQTT客户端对象
    MQTTClient m_client{ nullptr };
    Mutex m_client_mutex;
};
