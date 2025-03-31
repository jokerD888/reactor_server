#pragma once
#include "channel.h"
#include "event_loop.h"
#include "socket.h"

class TcpServer {
private:
    EventLoop loop_;

public:
    TcpServer(const std::string& ip, const uint16_t& port);
    ~TcpServer();
    void Start();
};