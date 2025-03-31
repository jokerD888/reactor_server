#pragma once
#include "acceptor.h"
#include "channel.h"
#include "connection.h"
#include "event_loop.h"
#include "socket.h"
class TcpServer {
private:
    EventLoop loop_;
    Acceptor* acceptor_;

public:
    TcpServer(const std::string& ip, const uint16_t& port);
    ~TcpServer();
    void Start();

    void NewConnection(Socket* client_sock);
};