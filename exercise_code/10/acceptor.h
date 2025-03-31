#pragma once

#include <functional>

#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket.h"

class Acceptor {
private:
    EventLoop* loop_;          // Acceptor 所在的 EventLoop,由构造函数传参
    Socket* serv_sock_;        // 服务端用于监听的 Socket，在构造函数中创建
    Channel* accept_channel_;  // Acceptor对应的 Channel，在构造函数中创建

public:
    Acceptor(EventLoop* loop, const std::string& ip, uint16_t port);
    ~Acceptor();
};