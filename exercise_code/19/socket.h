#pragma once

#include <errno.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "inet_address.h"

int CreateNonBlocking();

class Socket {
private:
    const int fd_;
    std::string ip_;  // 如果是listen_fd,存放服务端监听的ip，如果是客户端连接的fd，存放对端的ip
    uint16_t port_;

public:
    Socket(int sockfd = CreateNonBlocking()) : fd_(sockfd) {}
    ~Socket() { close(fd_); }

    inline int fd() const { return fd_; }
    std::string ip() const;
    uint16_t port() const;
    void SetReuseAddr(bool on = true);
    void SetReusePort(bool on = true);
    void SetTcpNoDelay(bool on = true);
    void SetKeepAlive(bool on = true);
    void SetIpPort(const std::string& ip, uint16_t port);
    void Bind(const InetAddress& serv_addr);
    void Listen(int n = 128);
    // client_addr 是输出参数
    int Accept(InetAddress& client_addr);
};
