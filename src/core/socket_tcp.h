#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <mutex>  // 新增：线程安全锁

// 跨平台Socket头文件
#ifdef _WIN64
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")  // Windows链接WSA库
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include "thread.h"

// 接收缓冲区大小
#define TCP_BUF_SIZE 4096
// 端口
#define TCP_PORT 9801

// 跨平台类型定义（统一socket句柄类型）
#ifdef _WIN64
typedef SOCKET SocketHandle;
const SocketHandle InvalidSocketHandle = INVALID_SOCKET;
#else
typedef int SocketHandle;
const SocketHandle InvalidSocketHandle = -1;
#endif

// TCP服务器类（补充客户端列表管理，支持广播发送）
class TcpServer {
public:
    TcpServer(uint16_t port = TCP_PORT);
    ~TcpServer();
    bool Start();                  // 启动服务器
    void Stop();                   // 停止服务器
    bool Broadcast(const std::string& data);  // 新增：向所有在线客户端广播数据
    int ClientCount(); // 获取客户端总数
    void SetServerPort(int port);

    /**
    * @brief 组装数据
    * @param source_str 源数据
    * @return 包装后的数据
    */
    std::string Packet(const std::string& source_str);

private:
    void AcceptLoop();             // 接受客户端连接循环
    void ClientManageLoop(SocketHandle sock, std::string clientIp, uint16_t clientPort);  // 客户端连接管理
    void AddClient(SocketHandle sock, const std::string& clientIp, uint16_t clientPort);  // 新增：添加客户端到列表
    void RemoveClient(SocketHandle sock);  // 新增：从列表移除客户端
    bool SendToClient(SocketHandle sock, const std::string& data);  // 新增：向单个客户端发送数据
    /**
     * @brief数据接收回调
     * @param recv_data 接收的数据
     */
    virtual void TcpServerRecv(const std::string& recv_data);

    /**
    * @brief 数据解包
    * @param source_str 源数据
    */
    void Unpack(const std::string& source_str);

private:
    uint16_t m_port;               // 监听端口
    SocketHandle m_listenSock;     // 监听socket
    std::atomic<bool> m_running{ false };   // 服务器运行状态（原子变量，线程安全）
    std::thread m_acceptThread;    // 接受连接线程

    // 新增：客户端列表及线程安全控制
    struct ClientInfo {
        SocketHandle sock;         // 客户端Socket
        std::string ip;            // 客户端IP
        uint16_t port;             // 客户端端口
    };
    std::vector<ClientInfo> m_clients;  // 在线客户端列表
    std::mutex m_clientMutex;           // 保护客户端列表的互斥锁
};

// TCP客户端类（修改Recv函数，无超时）
class TcpClient : protected Thread{
public:
    TcpClient();
    ~TcpClient();
    bool Connect(const std::string& ip, uint16_t port = TCP_PORT);  // 连接服务器
    void ConnectAsync(const std::string& ip, uint16_t port = TCP_PORT);  // 异步连接服务器
    void Disconnect();                                   // 断开连接
    bool Recv(std::string& buf);  // 修改：移除超时参数，无超时阻塞接收
    bool Send(const std::string& data); // 发送数据
    SocketHandle GetSocket() const { return m_sock; }   // 获取当前socket
    bool IsConnected() const { return m_connected; }    // 获取连接状态

private:
    /**
    * @brief 组装数据
    * @param source_str 源数据
    * @return 包装后的数据
    */
    std::string Packet(const std::string& source_str);

    /**
    * @brief 数据解包
    * @param source_str 源数据
    */
    void Unpack(const std::string& source_str);

    /**
     * @brief数据接收回调
     * @param recv_data 接收的数据
     */
    virtual void TcpClientRecv(const std::string& recv_data);

    /**
    * @brief 线程循环函数
    * @return true:线程循环函数继续执行；false:线程循环函数停止执行
    */
    bool ThreadLoop() override;

private:
    SocketHandle m_sock;           // 客户端socket
    std::atomic<bool> m_connected; // 连接状态
    // 服务器地址
    std::string m_server_addr;
    // 服务器端口号
    uint16_t m_port{ TCP_PORT };
    std::mutex m_sendMutex; // 新增：发送线程安全锁（多线程发送时使用）
};
