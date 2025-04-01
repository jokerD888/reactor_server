#include "socket.h"

int CreateNonBlocking() {
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0) {
        printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        exit(-1);
    }
    return listenfd;
}

void Socket::SetReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}
void Socket::SetReusePort(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}
void Socket::SetTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}
void Socket::SetKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

void Socket::Bind(const InetAddress& serv_addr) {
    if (bind(fd_, serv_addr.addr(), sizeof(sockaddr)) < 0) {
        perror("bind() failed\n");
        close(fd_);
        exit(-1);
    }
}
void Socket::Listen(int n) {
    if (listen(fd_, n) < 0) {
        perror("listen() failed\n");
        close(fd_);
        exit(-1);
    }
}
int Socket::Accept(InetAddress& client_addr) {
    sockaddr_in peer_addr;
    socklen_t len = sizeof(peer_addr);
    int client_fd = accept4(fd_, (sockaddr*)&peer_addr, &len, SOCK_NONBLOCK);

    client_addr.SetAddr(peer_addr);
    return client_fd;
}