#pragma once

#include <functional>

#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket.h"

class Connection {
private:
    EventLoop* loop_;          // Acceptor 所在的 EventLoop,由构造函数传参
    Socket* client_sock_;      // 服务端用于监听的 Socket，在构造函数中创建
    Channel* client_channel_;  // Acceptor对应的 Channel，在构造函数中创建
    std::function<void(Connection*)> close_cb_;
    std::function<void(Connection*)> error_cb_;

public:
    int fd() const;
    std::string ip() const;
    uint16_t port() const;
    Connection(EventLoop* loop, Socket* client_sock);
    ~Connection();

    void CloseCallback();  // TCP连接关闭（断开）的回调函数，供Channel调用
    void ErrorCallback();  // TCP连接出错的回调函数，供Channel调用
    inline void SetCloseCallback(std::function<void(Connection*)> cb) { close_cb_ = cb; }
    inline void SetErrorCallback(std::function<void(Connection*)> cb) { error_cb_; }
};