#pragma once
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "acceptor.h"
#include "channel.h"
#include "connection.h"
#include "event_loop.h"
#include "socket.h"
#include "thread_pool.h"
class TcpServer {
private:
    std::unique_ptr<EventLoop> main_loop_;               // 主事件循环
    std::vector<std::unique_ptr<EventLoop>> sub_loops_;  // 从属事件循环
    ThreadPool thread_pool_;
    int thread_num_;  // 线程池的大小也即为从事件的个数
    Acceptor acceptor_;
    std::map<int, std::shared_ptr<Connection>> conns_;
    std::function<void(std::shared_ptr<Connection>)> new_connection_cb_;
    std::function<void(std::shared_ptr<Connection>)> close_connection_cb_;
    std::function<void(std::shared_ptr<Connection>)> error_connection_cb_;
    std::function<void(std::shared_ptr<Connection>, std::string&)> message_cb_;
    std::function<void(EventLoop*)> epoll_timeout_cb_;
    std::function<void(std::shared_ptr<Connection>)> send_complete_cb_;

    std::mutex mutex_;  // 保护conns_的互斥锁

public:
    TcpServer(const std::string& ip, const uint16_t& port, int thread_num = 3);
    ~TcpServer();
    void Start();

    void NewConnection(std::unique_ptr<Socket> client_sock);
    void CloseConnection(std::shared_ptr<Connection> conn);  // 关闭客户端的连接，在Connection类中回调此函数
    void ErrorConnection(std::shared_ptr<Connection> conn);  // 客户端连接出错，在Connection类中回调此函数
    void Message(std::shared_ptr<Connection> conn, std::string& message);
    void SendComplete(std::shared_ptr<Connection> conn);
    void EpollTimeout(EventLoop* loop);

    void SetNewConnectionCallback(std::function<void(std::shared_ptr<Connection>)> cb) { new_connection_cb_ = cb; }
    void SetCloseConnectionCallback(std::function<void(std::shared_ptr<Connection>)> cb) { close_connection_cb_ = cb; }
    void SetErrorConnectionCallback(std::function<void(std::shared_ptr<Connection>)> cb) { error_connection_cb_ = cb; }
    void SetMessageCallback(std::function<void(std::shared_ptr<Connection>, std::string&)> cb) { message_cb_ = cb; }
    void SetSendCompleteCallback(std::function<void(std::shared_ptr<Connection>)> cb) { send_complete_cb_ = cb; }
    void SetEpollTimeoutCb(std::function<void(EventLoop*)> cb) { epoll_timeout_cb_ = cb; }
};