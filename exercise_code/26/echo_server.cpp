#include "echo_server.h"

EchoServer::EchoServer(const std::string& ip, uint16_t port, int sub_thread_num, int work_thread_num)
    : tcpserver_(ip, port, sub_thread_num), thread_pool_(work_thread_num, "WORKS") {
    tcpserver_.SetNewConnectionCallback(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.SetMessageCallback(
        std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.SetCloseConnectionCallback(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.SetErrorConnectionCallback(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
    tcpserver_.SetSendCompleteCallback(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    tcpserver_.SetEpollTimeoutCb(std::bind(&EchoServer::HandleEpollTimeout, this, std::placeholders::_1));
}
EchoServer ::~EchoServer() {}

void EchoServer::Start() { tcpserver_.Start(); }

void EchoServer::HandleNewConnection(std::shared_ptr<Connection> conn) {
    std::cout << "New connection from " << conn->ip() << ":" << conn->port() << std::endl;
}
void EchoServer::HandleClose(std::shared_ptr<Connection> conn) {
    std::cout << "Connection from " << conn->ip() << ":" << conn->port() << " closed" << std::endl;
}
void EchoServer::HandleError(std::shared_ptr<Connection> conn) {
    std::cout << "Error from " << conn->ip() << ":" << conn->port() << std::endl;
}

void EchoServer::OnMessage(std::shared_ptr<Connection> conn, std::string& message) {
    message = "reply: " + message;
    std::cout << "OnMessage" << (const char*)message.data() << std::endl;
    conn->Send(message.data(), message.size());
}
void EchoServer::HandleMessage(std::shared_ptr<Connection> conn, std::string& message) {
    if (thread_pool_.size() == 0) {
        // 如果没有工作线程，则直接在IO线程处理消息
        OnMessage(conn, message);
    } else {
        thread_pool_.AddTask(std::bind(&EchoServer::OnMessage, this, conn, message));
    }
}
void EchoServer::HandleSendComplete(std::shared_ptr<Connection> conn) {
    std::cout << "Send complete from " << conn->ip() << ":" << conn->port() << std::endl;
}
void EchoServer::HandleEpollTimeout(EventLoop* loop) { std::cout << "Epoll timeout" << std::endl; }