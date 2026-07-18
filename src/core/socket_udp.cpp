#include "socket_udp.h"
#include "log.h"
#include "base64.h"
#include "http_client.h"
#include "net_manager.h"

namespace {
    // 获取错误码
    static const int GetErrorNumber() {
#ifdef _WIN64
        return WSAGetLastError();
#else
        return errno;
#endif
    }
}

UdpServer::~UdpServer() {
    // 释放资源
    CLOSE_SOCKET(m_socket);
#ifdef _WIN64
    WSACleanup();
#endif
}

void UdpServer::SetServerIp(const std::string& ip) {
    m_ip_addr = ip;
}

void UdpServer::SetServerPort(int port) {
    m_port = port;
}

bool UdpServer::Start(const std::string& addr, int port) {
    // 将IP地址变为子网广播地址
    auto mask_addr = NetManager::ReplaceLastIpSegmentTo255(addr);
    SetServerIp(mask_addr);
    SetServerPort(port);
    return Start();
}

bool UdpServer::Start() {
#ifdef _WIN64
    // Windows初始化网络库
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOGE("WSAStartup failed! Error: %d\n", WSAGetLastError());
        return false;
    }
#endif

    // 1. 创建UDP套接字
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == INVALID_SOCKET) {
        LOGE("Create socket failed! Error: %d\n", GetErrorNumber());
        return false;;
    }

    // 2. 开启广播权限（必需，否则无法发送广播）
    int broadcast_en = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST,
        (const char*)&broadcast_en, sizeof(broadcast_en)) == SOCKET_ERROR) {
        LOGE("Set broadcast option failed! Error: %d\n", GetErrorNumber());
        return false;;
    }

    // 3. 配置广播目标地址（子网广播IP + 固定端口）
    memset(&m_broadcast_addr, 0, sizeof(m_broadcast_addr));
    m_broadcast_addr.sin_family = AF_INET;
    m_broadcast_addr.sin_port = htons(m_port);  // 与客户端监听端口一致

    // 转换子网广播IP为网络字节序
    if (inet_pton(AF_INET, m_ip_addr.c_str(), &m_broadcast_addr.sin_addr) <= 0) {
        LOGE("Invalid broadcast IP: %s\n", m_ip_addr.c_str());
        return false;;
    }

    LOGI("Start udp server addr:%s port:%d", m_ip_addr.c_str(), m_port);
    return true;
}

bool UdpServer::IsPrepared() {
    return (INVALID_SOCKET != m_socket);
}

int UdpServer::Broadcast(const std::string& data) {
    return sendto(m_socket, data.data(), data.size(), 0,
        (struct sockaddr*)&m_broadcast_addr, sizeof(m_broadcast_addr));
}

UdpClient::~UdpClient() {
    // 请求退出线程
    RequestQuit();

    // 释放资源
    CLOSE_SOCKET(m_socket);
#ifdef _WIN64
    WSACleanup();
#endif
}

void UdpClient::SetServerIp(const std::string& ip) {
    m_ip_addr = ip;
}

void UdpClient::SetServerPort(int port) {
    m_port = port;
}

bool UdpClient::Connect(const std::string& addr, int port) {
    SetServerIp(addr);
    SetServerPort(port);
    return Connect();
}

bool UdpClient::Connect() {
#ifdef _WIN64
    // Windows初始化网络库
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOGE("WSAStartup failed! Error: %d\n", WSAGetLastError());
        return false;
    }
#endif

    // 1. 创建UDP套接字
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == INVALID_SOCKET) {
        LOGE("Create socket failed! Error: %d\n", GetErrorNumber());
        return false;
    }

    // 2. 开启广播权限（必需，否则无法接收广播）
    int broadcast_en = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST,
        (const char*)&broadcast_en, sizeof(broadcast_en)) == SOCKET_ERROR) {
        LOGE("Set broadcast option failed! Error: %d\n", GetErrorNumber());
        return false;
    }

    // 3. 绑定本地地址（监听同一网段的广播）
    memset(&m_broadcast_addr, 0, sizeof(m_broadcast_addr));
    m_broadcast_addr.sin_family = AF_INET;
    m_broadcast_addr.sin_port = htons(m_port);  // 与服务器端口一致
#ifdef _WIN64
    // 转换客户端本地IP为网络字节序（INADDR_ANY表示监听所有网卡）
    if (inet_pton(AF_INET, m_ip_addr.c_str(), &m_broadcast_addr.sin_addr) <= 0) {
        LOGE("Invalid client local IP: %s\n", m_ip_addr.c_str());
        return false;
    }
#elif __linux__
    m_broadcast_addr.sin_addr.s_addr = INADDR_ANY;  // 监听所有网络接口
#endif

    // 绑定端口（必须绑定，否则无法接收指定端口的广播）
    if (bind(m_socket, (struct sockaddr*)&m_broadcast_addr, sizeof(m_broadcast_addr)) == SOCKET_ERROR) {
        LOGE("Bind port %d failed! Error: %d\n", m_port, GetErrorNumber());
        return false;
    }

    LOGI("Start udp client addr:%s port:%d", m_ip_addr.c_str(), m_port);
    // 启动线程接收数据
    Run();

    return true;
}

bool UdpClient::IsPrepared() {
    return (INVALID_SOCKET != m_socket);
}

bool UdpClient::ThreadLoop() {
    // 接收广播数据（阻塞等待）
    char buffer[UDP_BUF_SIZE]{0};
    struct sockaddr_in server_addr;  // 存储发送广播的服务器地址
    socklen_t server_addr_len = sizeof(server_addr);

    const int recv_len = recvfrom(m_socket, buffer, UDP_BUF_SIZE -1, 0,
        (struct sockaddr*)&server_addr, &server_addr_len);

    // 判断是否退出线程
    if (IsRequestQuit()) {
        LOGI("Exit broadcast recv function......");
        return false;
    }

    if (recv_len == SOCKET_ERROR) {
        LOGI("Receive failed! Error: %d\n", GetErrorNumber());
    }
    else {
        // 数据解包
        Unpack(std::string(buffer, recv_len));
    }
    // 继续执行线程函数
    return true;
}

void UdpClient::Unpack(const std::string& source_str) {
    std::string m_recv_buffer(source_str);

    // 最小长度：3个分隔符+MD5校验长度=35
    while (35 < m_recv_buffer.size()) {
        // 数据过长直接丢弃
        if (UDP_BUF_SIZE < m_recv_buffer.size()) {
            LOGI("Content too large:%d limit size:%d content:%s",
                m_recv_buffer.size(), UDP_BUF_SIZE, m_recv_buffer.c_str());
            m_recv_buffer.clear();
            break;
        }

        // 拆分数据（数据格式为：#数据信息的base64编码@数据信息的md5校验*）
        auto end_pos = m_recv_buffer.find("*");
        if (0 == end_pos) { // 丢失数据
            LOGI("Erase * character");
            m_recv_buffer.erase(m_recv_buffer.begin());
            continue;
        }
        else if (std::string::npos == end_pos) { // 一帧数据未接收完整
            LOGI("Not found * character.");
            break;
        }

        // 获取一帧数据
        std::string frame = m_recv_buffer.substr(0, end_pos);
        // 移除已处理数据
        m_recv_buffer = m_recv_buffer.substr(end_pos + 1); // 加1跳过*符号

        // 获取帧头
        auto head_pos = frame.find("#");
        if (std::string::npos == head_pos) {
            LOGI("Failed to get # character");
            continue;
        }
        // 丢弃#前的数据
        frame = frame.substr(head_pos);

        // 获数据和MD5校验分隔符
        auto split_pos = frame.find("@");
        if (std::string::npos == head_pos) {
            LOGI("Failed to get @ character");
            continue;
        }

        // 判断是否存在数据
        if (head_pos + 1 >= split_pos) {
            LOGW("Get msg failed,head_pos:%d split_pos:%d", head_pos, split_pos);
            continue;
        }
        // 获取数据信息
        std::string msg = frame.substr(head_pos + 1, split_pos - head_pos - 1);

        // 判断是否检验码长度正确
        if (33 != end_pos - split_pos) {
            LOGI("Get md5 checksum failed,split_pos:%d end_pos:%d", split_pos, end_pos);
            continue;
        }
        // 获取MD5校验信息
        const std::string md5_check_sum = frame.substr(split_pos + 1, 32);

        // 判断md5是否一致
        const std::string msg_md5 = Http::Md5Checksum(msg);
        if (md5_check_sum != msg_md5) {
            LOGI("MD5 not equal,md5_check_sum:%s msg_md5:%s", md5_check_sum.c_str(), msg_md5.c_str());
            continue;
        }

        // 分发接收到的广播消息
        const std::string msg_decode = base64_decode(msg);
        UdpClientRecv(msg_decode);
    }
}

void UdpClient::UdpClientRecv(const std::string& recv_data) {
    LOGI("Unsolved recv data:%s", recv_data.c_str());
}

