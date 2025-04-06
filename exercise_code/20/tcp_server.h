#pragma once
#include <map>

#include "acceptor.h"
#include "channel.h"
#include "connection.h"
#include "event_loop.h"
#include "socket.h"
class TcpServer {
private:
    EventLoop loop_;
    Acceptor* acceptor_;
    std::map<int, Connection*> conns_;
    std::function<void(Connection*)> new_connection_cb_;
    std::function<void(Connection*)> close_connection_cb_;
    std::function<void(Connection*)> error_connection_cb_;
    std::function<void(Connection*, std::string&)> message_cb_;
    std::function<void(EventLoop*)> epoll_timeout_cb_;
    std::function<void(Connection*)> send_complete_cb_;

public:
    TcpServer(const std::string& ip, const uint16_t& port);
    ~TcpServer();
    void Start();

    void NewConnection(Socket* client_sock);
    void CloseConnection(Connection* conn);  // 关闭客户端的连接，在Connection类中回调此函数
    void ErrorConnection(Connection* conn);  // 客户端连接出错，在Connection类中回调此函数
    void Message(Connection* conn, std::string& message);
    void SendComplete(Connection* conn);
    void EpollTimeout(EventLoop* loop);

    void SetNewConnectionCallback(std::function<void(Connection*)> cb) { new_connection_cb_ = cb; }
    void SetCloseConnectionCallback(std::function<void(Connection*)> cb) { close_connection_cb_ = cb; }
    void SetErrorConnectionCallback(std::function<void(Connection*)> cb) { error_connection_cb_ = cb; }
    void SetMessageCallback(std::function<void(Connection*, std::string&)> cb) { message_cb_ = cb; }
    void SetSendCompleteCallback(std::function<void(Connection*)> cb) { send_complete_cb_ = cb; }
    void SetEpollTimeoutCb(std::function<void(EventLoop*)> cb) { epoll_timeout_cb_ = cb; }
};