#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

class InetAddress {
private:
    sockaddr_in addr_;

public:
    InetAddress() = default;
    InetAddress(const std::string& ip, uint16_t port);
    InetAddress(const sockaddr_in& addr) : addr_(addr) {}
    inline void SetAddr(const sockaddr_in& addr) { addr_ = addr; }
    ~InetAddress();

    const char* ip() const;
    uint16_t port() const;
    const sockaddr* addr() const;
};