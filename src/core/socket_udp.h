#pragma once
#include <stdio.h>
#include <string.h>
#include <string>
#include <functional>

#include "thread.h"
#include "json_manager.h"

// 跨平台/Windows平台兼容
#ifdef _WIN64
#include <winsock2.h>
#include <ws2tcpip.h>  // 包含inet_pton
#pragma comment(lib, "ws2_32.lib")
#define SOCKET_T SOCKET
#define CLOSE_SOCKET closesocket
#elif __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET_T int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define CLOSE_SOCKET close
#endif

// 接收缓冲区大小
#define UDP_BUF_SIZE 4096
// 端口
#define UDP_PORT 9800

class UdpServer {
public:
    ~UdpServer();

    /**
     * @brief 设置IP
     */
    void SetServerIp(const std::string& ip);

    /**
     * @brief 设置端口
     */
    void SetServerPort(int port);

    /**
    * @brief 创建通信套接字
    * @param addr ip地址
    * @param port 端口
    * @return true 创建成功；false 创建失败
    */
    bool Start(const std::string& addr, int port = UDP_PORT);
private:
    bool Start();
public:
    /**
     * @brief 当前是否初始化
     */
    bool IsPrepared();

    /**
    * @brief 发送数据
    * @param data 数据
    * @return 发送的数据长度
    */
    int Broadcast(const std::string& data);

private:
    // 套接字fd
    SOCKET_T m_socket{ INVALID_SOCKET };
    // IP
    std::string m_ip_addr;
    // 端口
    int m_port{ UDP_PORT };
    // 广播地址
    struct sockaddr_in m_broadcast_addr { 0 };
};

class UdpClient : public Thread {
public:
    ~UdpClient();

    /**
     * @brief 设置IP
     */
    void SetServerIp(const std::string& ip);

    /**
     * @brief 设置端口
     */
    void SetServerPort(int port);

    /**
    * @brief 创建通信套接字
    * @param addr ip地址
    * @param port 端口
    * @return true 客户端启动成功；false 客户端启动失败
    */
    bool Connect(const std::string& addr, int port = UDP_PORT);
    bool Connect();
public:
    /**
     * @brief 当前是否初始化
     */
    bool IsPrepared();

private:
    /**
    * @brief 线程循环函数
    * @return true:线程循环函数继续执行；false:线程循环函数停止执行
    */
    bool ThreadLoop() override;

    /**
    * @brief 数据解包
    * @param source_str 接收到的数据
    */
    virtual void Unpack(const std::string& source_str);

    /**
     * @brief 分发接收到的数据
     * @param recv_data 接收到的数据(解析后)
     */
    virtual void UdpClientRecv(const std::string& recv_data);

protected:
    // 套接字fd
    SOCKET_T m_socket{ INVALID_SOCKET };
    // IP
    std::string m_ip_addr{ "0.0.0.0" };
    // 端口
    int m_port{ UDP_PORT };
    // 广播地址
    struct sockaddr_in m_broadcast_addr { 0 };
};
