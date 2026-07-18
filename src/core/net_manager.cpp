#include "net_manager.h"
#include "log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")


namespace {
/**
* 获取本机所有 IPv4 地址（排除回环地址）
* @param ips 存储结果的数组（需提前分配内存，建议至少 10 个元素）
* @param max_count 数组最大容量
* @return 实际获取的 IP 数量
*/
static int GetLocalIpv4(char** ips, int max_count) {
    if (ips == NULL || max_count <= 0) {
        return 0;
    }

    int count = 0;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 0;
    }

    ULONG outBufLen = 1500;
    PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
    if (pAddresses == NULL) {
        WSACleanup();
        return 0;
    }

    DWORD ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_ALL_INTERFACES, NULL, pAddresses, &outBufLen);
    if (ret == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == NULL) {
            WSACleanup();
            return 0;
        }
        ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_ALL_INTERFACES, NULL, pAddresses, &outBufLen);
    }

    if (ret != NO_ERROR) {
        printf("GetAdaptersAddresses failed: %d\n", ret);
        free(pAddresses);
        WSACleanup();
        return 0;
    }

    PIP_ADAPTER_ADDRESSES pCurrAddr = pAddresses;
    while (pCurrAddr != NULL && count < max_count) {
        if (pCurrAddr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
            pCurrAddr = pCurrAddr->Next;
            continue;
        }

        PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddr->FirstUnicastAddress;
        while (pUnicast != NULL && count < max_count) {
            sockaddr* sa = pUnicast->Address.lpSockaddr;
            char ip_str[INET_ADDRSTRLEN] = { 0 };

            if (sa->sa_family == AF_INET) {
                sockaddr_in* sin = (sockaddr_in*)sa;
                inet_ntop(AF_INET, &sin->sin_addr, ip_str, sizeof(ip_str));
                if (strcmp(ip_str, "127.0.0.1") != 0) {
                    ips[count] = strdup(ip_str);
                    count++;
                }
            }

            pUnicast = pUnicast->Next;
        }

        pCurrAddr = pCurrAddr->Next;
    }

    free(pAddresses);
    WSACleanup();

    return count;
}
}

std::vector<std::string> NetManager::GetIpAddr() {
    std::vector<std::string> ip_addr_vec;

    const int MAX_IP_COUNT = 10;
    char* ips[MAX_IP_COUNT] = { NULL };

    const int ip_count = GetLocalIpv4(ips, MAX_IP_COUNT);
    if (ip_count == 0) {
        LOGW("No valid IPv4 addresses found.\n");
        return ip_addr_vec;
    }

    for (int i = 0; i < ip_count; i++) {
        if (ips[i]) {
            ip_addr_vec.push_back(std::string(ips[i], strlen(ips[i])));
        }
        free(ips[i]);
    }

    return ip_addr_vec;
}

bool NetManager::GetConnectStatus() {
    auto vec = GetIpAddr();
    return (0 < vec.size());
}

std::string NetManager::ReplaceLastIpSegmentTo255(const std::string& ip_str) {
    std::vector<std::string> segments;
    std::stringstream ss(ip_str);
    std::string seg;

    while (std::getline(ss, seg, '.')) {
        segments.push_back(seg);
    }

    if (segments.size() != 4) {
        LOGE("The ip not 4 segment.");
        return "";
    }

    for (const auto& segment : segments) {
        try {
            int num = std::stoi(segment);
            if (num < 0 || num > 255) {
                LOGE("The ip value not range in 0 ~ 255");
                return "";
            }
        }
        catch (...) {
            LOGE("Format error.");
            return "";
        }
    }

    return segments[0] + "." + segments[1] + "." + segments[2] + ".255";
}

std::string NetManager::GetBroadcastAddr() {
    std::vector<std::string> ip_vec = GetIpAddr();
    if (0 == ip_vec.size()) {
        LOGI("Failed to get broadcast ip.");
        return "0.0.0.255";
    }
    return ReplaceLastIpSegmentTo255(ip_vec[0]);
}

std::string NetManager::IpAddr() {
    std::vector<std::string> ip_vec = GetIpAddr();
    if (0 == ip_vec.size()) {
        LOGI("Failed to get broadcast ip.");
        return "";
    }
    return ip_vec[0];
}
