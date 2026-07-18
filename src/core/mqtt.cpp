#include "mqtt.h"
#include "log.h"
#include "string_helper.h"
#include "net_manager.h"

#include <cstring>


namespace {
    #define QOS         1
    #define TIMEOUT     10000L

    static void delivered(void* context, MQTTClient_deliveryToken dt)
    {
        LOGI("Message with token value %d delivery confirmed\n", dt);
    }

    static int msgarrvd(void* context, char* topicName, int topicLen, MQTTClient_message* message)
    {
        LOGI("Message arrived\n");
        LOGI("     topic: %s", topicName);
        LOGI("   message: %.*s\n", message->payloadlen, (char*)message->payload);

        // 执行数据回调
        if (context) {
            MqttMsg msg;
            msg.m_topic.assign(topicName, strlen(topicName));
            msg.m_message.assign((char*)message->payload, message->payloadlen);
            ((Mqtt*)context)->ExecuteMsgRecvCallback(msg);
        }

        MQTTClient_freeMessage(&message);
        MQTTClient_free(topicName);
        return 1;
    }

    static void connlost(void* context, char* cause)
    {
        LOGI("\nConnection lost\n");
        if (cause)
            LOGI("     cause: %s\n", cause);

        // 标记断开与MQTT服务器的连接
        if (context) {
            ((Mqtt*)context)->SetConnectStatus(false);
        }
    }

    // 日志回调函数：打印Paho内部日志
    static void MqttLogCallback(enum MQTTCLIENT_TRACE_LEVELS level, char* message) {
        LOGI("[MQTT LOG] level=%d: %s", level, message);
    }
}

bool Mqtt::ThreadLoop() {
    // 网络断开不执行MQTT连接操作
    while (!NetManager::GetConnectStatus()) {
        LOGI("Net is disconnected!!!.");
        Sleep(3 * 1000);
    }

    // 设置日志回调（开启详细日志）
    //MQTTClient_setTraceCallback(MqttLogCallback);

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc{ MQTTCLIENT_FAILURE };

    std::string mqtthost;
    mqtthost.append(m_mqtt_host);
    mqtthost.append(":");
    mqtthost.append(m_mqtt_port);
    LOGI("Using server at %s\n", mqtthost.c_str());

    if ((rc = MQTTClient_create(&m_client, mqtthost.c_str(), m_client_id.c_str(),
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        LOGI("Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto exit;
    }

    if ((rc = MQTTClient_setCallbacks(m_client, this, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        LOGI("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;
    conn_opts.username = m_user_name.c_str();   // 设置用户名
    conn_opts.password = m_password.c_str();    // 设置密码
    if ((rc = MQTTClient_connect(m_client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        LOGI("Failed to connect, return code %d", rc);
        LOGI("user name %s password %s", conn_opts.username, conn_opts.password);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    // 注册mqtt主题
    for (auto topic : m_topic_vec) {
        LOGI("Subscribing to topic %s for client %s using QoS%d", topic.c_str(), m_client_id.c_str(), QOS);
        if ((rc = MQTTClient_subscribe(m_client, topic.c_str(), QOS)) != MQTTCLIENT_SUCCESS)
        {
            LOGI("Failed to subscribe, return code %d\n", rc);
            rc = EXIT_FAILURE;
            goto destroy_exit;
        }
    }

    // 标记已连接MQTT服务器
    SetConnectStatus(true);

    // 连接上服务器,执行回调
    ExecuteConnServerCallback();

    // 死循环执行MQTT客户端代码
    while (GetConnectStatus()) { Sleep(5 * 1000); }

destroy_exit:
    MQTTClient_destroy(&m_client);
exit:
    LOGE("Reconnect mqtt server. rc:%d", rc);
    // 延时2秒
    Thread::Sleep(2*1000);
    return true;
}

void Mqtt::SetMqttHost(const std::string& host) {
    m_mqtt_host = host;
}

std::string Mqtt::GetMqttHost() {
    return m_mqtt_host;
}

void Mqtt::SetMqttPort(const std::string& port) {
    m_mqtt_port = port;
}

std::string Mqtt::GetMqttPort() {
    return m_mqtt_port;
}

void Mqtt::SetMqttUserName(const std::string& user_name) {
    m_user_name = user_name;
}

std::string Mqtt::GetMqttUserName() {
    return m_user_name;
}

void Mqtt::SetMqttUserPassword(const std::string& password) {
    m_password = password;
}

std::string Mqtt::GetMqttUserPassword() {
    return m_password;
}

void Mqtt::SetClientId(const std::string& client_id) {
    m_client_id = client_id;
}

void Mqtt::SetTopic(const std::string& topic) {
    std::vector<std::string> topic_vec;
    topic_vec.push_back(topic);
    SetTopic(topic_vec);
}

void Mqtt::SetTopic(const std::vector<std::string>& topic) {
    m_topic_vec = topic;
}

void Mqtt::AppendTopic(const std::string& topic) {
    m_topic_vec.push_back(topic);
}

void Mqtt::ConnectServer() {
    Run();
}

bool Mqtt::Publish(const std::string& topic, const std::string& message) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc = MQTTCLIENT_FAILURE;

    pubmsg.payload = (void*)message.c_str();
    pubmsg.payloadlen = message.size();
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    
    // 加锁,避免多线程发送竞争
    AutoLock _l(m_client_mutex); 
    if ((rc = MQTTClient_publishMessage(m_client, topic.c_str(), &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
        LOGE("Failed to publish message, return code %d\n", rc);
        return false;
    }
    //LOGI("\npublish topic:%s\nmessage:%s\nrc:%d", topic.c_str(), message.c_str(), rc);

    return (MQTTCLIENT_SUCCESS == rc);
}

void Mqtt::AddMsgRecvCallback(MsgReceive callback) {
    m_recv_callback = callback;
}

void Mqtt::ExecuteMsgRecvCallback(const MqttMsg& msg) {
    if (m_recv_callback) {
        m_recv_callback(msg);
    }
}

void Mqtt::AddConnServerCallback(ConnServer callback) {
    m_conn_server_callback = callback;
}

void Mqtt::ExecuteConnServerCallback() {
    if (m_conn_server_callback) {
        m_conn_server_callback();
    }
}

void Mqtt::SetConnectStatus(bool status) {
    m_connect_status = status;
}

bool Mqtt::GetConnectStatus() {
    return m_connect_status;
}
