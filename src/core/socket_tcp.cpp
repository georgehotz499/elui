#include "socket_tcp.h"

#include "log.h"
#include "base64.h"
#include "http_client.h"

#include <chrono>
#include <cstddef>


namespace {
    // 跨平台关闭Socket
    static inline void CloseSocket(SocketHandle sock) {
#ifdef _WIN64
        closesocket(sock);
#else
        close(sock);
#endif
    }

    // 跨平台初始化（Windows初始化WSA，Linux无操作）
    static inline bool InitNetwork() {
#ifdef _WIN64
        WSADATA wsaData;
        int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (ret != 0) {
            LOGE("Failed to init WSA, error num %d", ret);
            return false;
        }
#endif
        return true;
    }

    // 跨平台清理（Windows清理WSA，Linux无操作）
    static inline void CleanupNetwork() {
#ifdef _WIN64
        WSACleanup();
#endif
    }

    // 跨平台错误信息获取
    static inline std::string GetLastErrorNum() {
        char err_buf[256] = { 0 };
#ifdef _WIN64
        int err_code = WSAGetLastError();
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            err_buf, sizeof(err_buf), NULL);
#else
        strerror_r(errno, err_buf, sizeof(err_buf));  // 线程安全的错误信息获取
#endif
        return std::string(err_buf);
    }

}

// -------------------------- TcpServer实现（补充客户端管理+广播） --------------------------
TcpServer::TcpServer(uint16_t port)
    : m_port(port), m_listenSock(InvalidSocketHandle), m_running(false) {
}

TcpServer::~TcpServer() {
    Stop();
    CleanupNetwork();
}

bool TcpServer::Start() {
    if (!InitNetwork()) {
        return false;
    }

    // 创建监听socket
    m_listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenSock == InvalidSocketHandle) {
        LOGE("Failed to create socket,error num %s", GetLastErrorNum().c_str());
        return false;
    }

#ifdef _WIN64
    // 设置端口复用（避免端口占用）
    BOOL reuse = TRUE;
    setsockopt(m_listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
    // 设置消息立即发送
    BOOL nodelay = TRUE;
    setsockopt(m_listenSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
#else
    // 设置端口复用（避免端口占用）
    int reuse = 1;
    setsockopt(m_listenSock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &reuse, sizeof(reuse));
    // 设置消息立即发送
    int nodelay = 1;
    setsockopt(m_listenSock, IPPROTO_TCP, O_NDELAY, &nodelay, sizeof(nodelay));
#endif

    // 绑定端口
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);          // 主机字节序→网络字节序
    addr.sin_addr.s_addr = INADDR_ANY;      // 监听所有网卡

    if (bind(m_listenSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        LOGE("Failed to bind port,error num %s", GetLastErrorNum().c_str());
        CloseSocket(m_listenSock);
        m_listenSock = InvalidSocketHandle;
        return false;
    }

    // 开始监听（最大等待队列10）
    if (listen(m_listenSock, 10) == -1) {
        LOGE("Failed to listen,error num %s", GetLastErrorNum().c_str());
        CloseSocket(m_listenSock);
        m_listenSock = InvalidSocketHandle;
        return false;
    }

    m_running = true;
    // 启动接收连接线程
    m_acceptThread = std::thread(&TcpServer::AcceptLoop, this);
    LOGI("Succeesed to listen port %d", m_port);
    return true;
}

void TcpServer::Stop() {
    if (!m_running) {
        return;
    }

    m_running = false;
    // 关闭监听socket，唤醒accept阻塞
    if (m_listenSock != InvalidSocketHandle) {
        CloseSocket(m_listenSock);
        m_listenSock = InvalidSocketHandle;
    }

    // 关闭所有客户端连接（线程安全）
    {
        std::lock_guard<std::mutex> lock(m_clientMutex);
        for (const auto& client : m_clients) {
            CloseSocket(client.sock);
            LOGI("Disconnected client %s port %d", client.ip.c_str(), client.port);
        }
        m_clients.clear();
    }

    // 等待接受线程退出
    if (m_acceptThread.joinable()) {
        m_acceptThread.join();
    }
    LOGI("The server had stop.");
}

// 新增：向所有在线客户端广播数据
bool TcpServer::Broadcast(const std::string& data) {
    if (!m_running || data.empty()) {
        LOGE("Failed to send data,m_running:%d data size:%d", static_cast<int>(m_running), data.size());
        return false;
    }

    std::lock_guard<std::mutex> lock(m_clientMutex);
    if (m_clients.empty()) {
        LOGI("Haven't any client had connected.");
        return false;
    }

    LOGI("Clent count %d,data:%d", m_clients.size(), data.size());
    // 遍历所有客户端发送数据，失败则移除离线客户端
    for (auto it = m_clients.begin(); it != m_clients.end();) {
        if (!SendToClient(it->sock, data)) {
            // 发送失败，说明客户端已离线，移除之
            LOGI("The client %s %d had disconnected.", it->ip.c_str(), it->port);
            CloseSocket(it->sock);
            it = m_clients.erase(it);
        }
        else {
            ++it;
        }
    }

    return true;
}

int TcpServer::ClientCount() {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    return m_clients.size();
}

void TcpServer::SetServerPort(int port) {
    m_port = port;
}

// 新增：向单个客户端发送数据（内部使用，不暴露给外部）
bool TcpServer::SendToClient(SocketHandle sock, const std::string& data) {
    if (sock == InvalidSocketHandle) {
        return false;
    }

    // 发送数据（TCP流式协议，确保完整发送）
    int sentLen = 0;
    const char* buf = data.c_str();
    size_t totalLen = data.size();

    while (sentLen < totalLen) {
        int ret = send(sock, buf + sentLen, totalLen - sentLen, 0);
        if (ret == -1) {
            LOGE("Failed to send to %d,error num %s", static_cast<int>(sock), GetLastErrorNum().c_str());
            return false;
        }
        sentLen += ret;
    }

    // 打印单个客户端发送成功日志（可选）
    // std::cout << "发送给Socket：" << sock << " 成功（长度：" << sentLen << "）" << std::endl;
    return true;
}

void TcpServer::TcpServerRecv(const std::string& recv_data) {
    LOGI("Unsolved broadcast package.\n%s", recv_data.c_str());
}

std::string TcpServer::Packet(const std::string& source_str) {
    // 数据格式为：#数据信息的base64编码@数据信息的md5校验*
    std::string package("#");

    std::string base64_str = base64_encode(source_str.data(), source_str.size());
    if (0 == base64_str.size()) {
        LOGW("Failed to base64 encode: size:%d content:%s", source_str.size(), source_str.c_str());
        return "";
    }
    std::string md5_str = Http::Md5Checksum(base64_str);

    package.append(base64_str).append("@").append(md5_str).append("*");
    return package;
}

void TcpServer::Unpack(const std::string& source_str) {
    std::string m_recv_buffer(source_str);

    // 最小长度：3个分隔符+MD5校验长度=35
    while (35 < m_recv_buffer.size()) {
        // 数据过长直接丢弃
        if (TCP_BUF_SIZE < m_recv_buffer.size()) {
            LOGI("Content too large:%d limit size:%d content:%s",
                m_recv_buffer.size(), TCP_BUF_SIZE, m_recv_buffer.c_str());
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
        TcpServerRecv(msg_decode);
    }
}

void TcpServer::ClientManageLoop(SocketHandle sock, std::string clientIp, uint16_t clientPort) {
    char recv_buf[1024] = { 0 };
    while (m_running) {
        int ret = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
        LOGI("Recv ret %d", ret);
        if (ret <= 0) {
            if (ret == 0) {
                LOGI("The client disconnected.");
            }
            else {
                // 非阻塞模式下EAGAIN/EWOULDBLOCK是正常的，忽略
#ifdef _WIN64
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
#else
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
#endif
                    LOGI("Connected to the client %s %d occur a exception,error num %s", clientIp.c_str(), clientPort, GetLastErrorNum().c_str());
                }
                }
            // 客户端主动断开连接
            break;
            }
        // 接收到数据
        else {
            Unpack(recv_buf);
        }

        // 每隔1秒检测一次连接状态
        std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    LOGI("Remove socket %d ip %s port %d", static_cast<int>(sock), clientIp.c_str(), clientPort);
    // 客户端离线：移除列表+关闭Socket
    RemoveClient(sock);
    CloseSocket(sock);
}

// 新增：添加客户端到列表（线程安全）
void TcpServer::AddClient(SocketHandle sock, const std::string& clientIp, uint16_t clientPort) {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    m_clients.push_back({ sock, clientIp, clientPort });
    LOGI("Client %s %d connected,client count %d", clientIp.c_str(), clientPort, m_clients.size());
}

// 新增：从列表移除客户端（线程安全）
void TcpServer::RemoveClient(SocketHandle sock) {
    std::lock_guard<std::mutex> lock(m_clientMutex);
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it->sock == sock) {
            LOGI("Client %s %d had remove,client count %d", it->ip.c_str(), it->port, m_clients.size()-1);
            m_clients.erase(it);
            break;
        }
    }
}

void TcpServer::AcceptLoop() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (m_running) {
        // 接受客户端连接（阻塞）
        SocketHandle clientSock = accept(m_listenSock, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSock == InvalidSocketHandle) {
            if (m_running) {  // 非主动停止时打印错误
                LOGE("Failed to accept.");
            }
            continue;
        }

        // 解析客户端IP和端口
        char clientIp[INET_ADDRSTRLEN] = { 0 };
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, sizeof(clientIp));
        uint16_t clientPort = ntohs(clientAddr.sin_port);

        // 添加客户端到列表（线程安全）
        AddClient(clientSock, clientIp, clientPort);

        // 启动独立线程管理客户端连接（检测断开）
        std::thread clientThread(&TcpServer::ClientManageLoop, this, clientSock, clientIp, clientPort);
        clientThread.detach();  // 分离线程（自动回收）
    }
}

// -------------------------- TcpClient实现（仅修改Recv函数） --------------------------
TcpClient::TcpClient() : m_sock(InvalidSocketHandle), m_connected(false) {}

TcpClient::~TcpClient() {
    Disconnect();
    CleanupNetwork();
}

bool TcpClient::Connect(const std::string& ip, uint16_t port) {
    // 已连接则先断开
    if (m_connected) {
        Disconnect();
    }

    if (!InitNetwork()) {
        return false;
    }

    // 创建客户端socket
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock == InvalidSocketHandle) {
        LOGE("Failed to create socket,error num %s", GetLastErrorNum().c_str());
        return false;
    }

    // 设置服务器地址
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // 解析IP地址（支持IPv4）
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        LOGE("Invalid addr %s", ip.c_str());
        CloseSocket(m_sock);
        m_sock = InvalidSocketHandle;
        return false;
    }

    // 连接服务器
    if (::connect(m_sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        LOGE("Failed to connect server %s %d,error num %s", ip.c_str(), port, GetLastErrorNum().c_str());
        CloseSocket(m_sock);
        m_sock = InvalidSocketHandle;
        return false;
    }

    m_connected = true;
    LOGI("Succeeded to connect server %s %d", ip.c_str(), port);
    return true;
}

void TcpClient::ConnectAsync(const std::string& ip, uint16_t port) {
    // 保存服务器地址和端口号
    m_server_addr = ip;
    m_port = port;
    // 启动线程连接服务器
    Run();
}

void TcpClient::Disconnect() {
    if (m_connected && m_sock != InvalidSocketHandle) {
        CloseSocket(m_sock);
        m_sock = InvalidSocketHandle;
        m_connected = false;
        LOGI("Had disconnected from server.");
    }
}

// 修改后：无超时阻塞接收数据
bool TcpClient::Recv(std::string& buf) {
    buf.clear();  // 清空输出缓冲区

    if (!m_connected || m_sock == InvalidSocketHandle) {
        LOGI("Haven't connected to server,failed to recv.");
        return false;
    }

    // 关键：移除超时设置，使用默认的阻塞模式recv
    char recvBuf[1024] = { 0 };
    // 阻塞调用：直到收到数据（>0）、连接断开（=0）或出错（-1）
    int recvLen = recv(m_sock, recvBuf, sizeof(recvBuf) - 1, 0);

    if (recvLen <= 0) {
        if (recvLen == 0) {
            LOGI("The server had disconnected.");
            Disconnect();  // 连接断开，清理资源
        }
        else {
            LOGI("Failed to recv data.");
        }
        return false;
    }

    // 拷贝接收数据到输出缓冲区
    buf.assign(recvBuf, recvLen);
    LOGI("Had recv data size %d", recvLen);
    return true;
}

bool TcpClient::Send(const std::string& data) {
    // 1. 前置检查：是否连接、数据是否为空
    if (!m_connected || m_sock == InvalidSocketHandle || data.empty()) {
        std::cerr << "发送失败：未连接服务器或数据为空" << std::endl;
        LOGE("Failed to send data,m_connected:%d data size:%d", static_cast<int>(m_connected), data.size());
        return false;
    }

    // 2. 线程安全：多线程发送时加锁（避免数据错乱）
    std::lock_guard<std::mutex> lock(m_sendMutex);
    // 数据打包
    std::string packeted_data = Packet(data);
    const char* buf = packeted_data.c_str();
    size_t totalLen = packeted_data.size();
    int sentLen = 0;

    // 3. 循环发送：直到所有数据发送完毕（应对 send 部分发送）
    while (sentLen < totalLen) {
        int ret = send(
            m_sock,                // 客户端socket
            buf + sentLen,         // 未发送的数据起始地址
            totalLen - sentLen,    // 剩余待发送长度
            0                      // 标志位（0=阻塞发送，推荐默认）
        );

        // 4. 错误处理
        if (ret == -1) {
            LOGE("Failed to send,error num %s", GetLastErrorNum().c_str());
            Disconnect();  // 发送失败（如连接断开），主动清理连接
            return false;
        }

        sentLen += ret;  // 累加已发送长度
    }

    // 5. 发送成功日志
    LOGI("Succeeded to send %d byte data.", sentLen);
    return true;

}

std::string TcpClient::Packet(const std::string& source_str) {
    // 数据格式为：#数据信息的base64编码@数据信息的md5校验*
    std::string package("#");

    std::string base64_str = base64_encode(source_str.data(), source_str.size());
    if (0 == base64_str.size()) {
        LOGW("Failed to base64 encode: size:%d content:%s", source_str.size(), source_str.c_str());
        return "";
    }
    std::string md5_str = Http::Md5Checksum(base64_str);

    package.append(base64_str).append("@").append(md5_str).append("*");
    return package;
}

void TcpClient::Unpack(const std::string& source_str) {
    std::string m_recv_buffer(source_str);

    // 最小长度：3个分隔符+MD5校验长度=35
    while (35 < m_recv_buffer.size()) {
        // 数据过长直接丢弃
        if (TCP_BUF_SIZE < m_recv_buffer.size()){
            LOGI("Content too large:%d limit size:%d content:%s",
                m_recv_buffer.size(), TCP_BUF_SIZE, m_recv_buffer.c_str());
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
        TcpClientRecv(msg_decode);
    }
}

void TcpClient::TcpClientRecv(const std::string& recv_data) {
    LOGI("Unsolved broadcast package.\n%s", recv_data.c_str());
}

bool TcpClient::ThreadLoop() {
    // 连接服务器
    if (!Connect(m_server_addr, m_port)) {
        LOGI("Failed to connect server.");
        // 延时2秒
        Thread::Sleep(3000);
        return !IsRequestQuit();
    }

    // 接收数据
    std::string recv_buf;
    while (IsConnected()) {
        // 无超时阻塞接收：直到收到数据或连接断开
        if (Recv(recv_buf)) {
            // 数据解包
            Unpack(recv_buf);
        }
        // 若Recv返回false，说明连接已断开或出错，循环会退出
        Thread::Sleep(50);
    }

    return !IsRequestQuit();
}
