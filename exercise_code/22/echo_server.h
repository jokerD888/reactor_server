#pragma once

#include "connection.h"
#include "event_loop.h"
#include "tcp_server.h"
#include "thread_pool.h"

class EchoServer {
public:
    TcpServer tcpserver_;
    ThreadPool thread_pool_;

public:
    EchoServer(const std::string& ip, uint16_t port, int sub_thread_num = 3, int work_thread_num = 5);
    ~EchoServer();

    void Start();

    void HandleNewConnection(Connection* client_sock);
    void HandleClose(Connection* conn);  // 关闭客户端的连接，在TcpServer类中回调此函数
    void HandleError(Connection* conn);  // 客户端连接出错，在TcpServer类中回调此函数
    void HandleMessage(Connection* conn, std::string& message);
    void HandleSendComplete(Connection* conn);
    void HandleEpollTimeout(EventLoop* loop);
    void OnMessage(Connection* conn, std::string& message);  // 处理客户端的请求报文，用于添加给线程池
};