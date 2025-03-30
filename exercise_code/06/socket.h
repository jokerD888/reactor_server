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

public:
    Socket(int sockfd = CreateNonBlocking()) : fd_(sockfd) {}
    ~Socket() { close(fd_); }

    inline int fd() const { return fd_; }
    void SetReuseAddr(bool on = true);
    void SetReusePort(bool on = true);
    void SetTcpNoDelay(bool on = true);
    void SetKeepAlive(bool on = true);
    void Bind(const InetAddress& serv_addr);
    void Listen(int n = 128);
    // client_addr 是输出参数
    int Accept(InetAddress& client_addr);
};
