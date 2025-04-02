#pragma once

#include <functional>

#include "buffer.h"
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
    std::function<void(Connection*, std::string)> message_cb_;

    Buffer input_buffer_;   // 接收缓存区
    Buffer output_buffer_;  // 发送缓存区

public:
    int fd() const;
    std::string ip() const;
    uint16_t port() const;
    Connection(EventLoop* loop, Socket* client_sock);
    ~Connection();

    void OnMessage();  // 处理对端发过来的消息

    void CloseCallback();  // TCP连接关闭（断开）的回调函数，供Channel调用
    void ErrorCallback();  // TCP连接出错的回调函数，供Channel调用
    void WriteCallback();  // 处理写事件的回调函数，供Channel调用
    inline void SetCloseCallback(std::function<void(Connection*)> cb) { close_cb_ = cb; }
    inline void SetErrorCallback(std::function<void(Connection*)> cb) { error_cb_; }
    inline void SetMessageCallback(std::function<void(Connection*, std::string)> cb) { message_cb_ = cb; }

    void Send(const char* data, size_t size);
};