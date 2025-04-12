#pragma once

#include <atomic>
#include <functional>
#include <memory>

#include "buffer.h"
#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket.h"
#include "time_stamp.h"

class EventLoop;
class Channel;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    EventLoop* loop_;                          // Acceptor 所在的 EventLoop,由构造函数传参
    std::unique_ptr<Socket> client_sock_;      // 服务端用于监听的 Socket，在构造函数中创建
    std::unique_ptr<Channel> client_channel_;  // Acceptor对应的 Channel，在构造函数中创建
    std::function<void(std::shared_ptr<Connection>)> close_cb_;
    std::function<void(std::shared_ptr<Connection>)> error_cb_;
    std::function<void(std::shared_ptr<Connection>, std::string&)> message_cb_;
    std::function<void(std::shared_ptr<Connection>)> send_complete_cb_;

    Buffer input_buffer_;           // 接收缓存区
    Buffer output_buffer_;          // 发送缓存区
    std::atomic_bool is_shutdown_;  // 客户端连接断开标志位

    TimeStamp last_time_;  // 上次接收数据的时间

public:
    int fd() const;
    std::string ip() const;
    uint16_t port() const;
    Connection(EventLoop* loop, std::unique_ptr<Socket> client_sock);
    ~Connection();

    void OnMessage();  // 处理对端发过来的消息

    void CloseCallback();  // TCP连接关闭（断开）的回调函数，供Channel调用
    void ErrorCallback();  // TCP连接出错的回调函数，供Channel调用
    void WriteCallback();  // 处理写事件的回调函数，供Channel调用
    inline void SetCloseCallback(std::function<void(std::shared_ptr<Connection>)> cb) { close_cb_ = cb; }
    inline void SetErrorCallback(std::function<void(std::shared_ptr<Connection>)> cb) { error_cb_; }
    inline void SetMessageCallback(std::function<void(std::shared_ptr<Connection>, std::string&)> cb) {
        message_cb_ = cb;
    }

    inline void SetSendCompleteCallback(std::function<void(std::shared_ptr<Connection>)> cb) { send_complete_cb_ = cb; }

    void Send(const char* data, size_t size);
    // void SendInLoop(const char* data, size_t size);
    void SendInLoop(std::string data);

    bool IsTimeout(time_t now, int val);
};