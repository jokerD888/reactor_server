#include "echo_server.h"

EchoServer::EchoServer(const std::string& ip, uint16_t port) : tcpserver_(ip, port) {
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

void EchoServer::HandleNewConnection(Connection* conn) {
    std::cout << "New connection from " << conn->ip() << ":" << conn->port() << std::endl;
}
void EchoServer::HandleClose(Connection* conn) {
    std::cout << "Connection from " << conn->ip() << ":" << conn->port() << " closed" << std::endl;
}
void EchoServer::HandleError(Connection* conn) {
    std::cout << "Error from " << conn->ip() << ":" << conn->port() << std::endl;
}
void EchoServer::HandleMessage(Connection* conn, std::string& message) {
    message = "reply: " + message;
    conn->Send(message.data(), message.size());
}
void EchoServer::HandleSendComplete(Connection* conn) {
    std::cout << "Send complete from " << conn->ip() << ":" << conn->port() << std::endl;
}
void EchoServer::HandleEpollTimeout(EventLoop* loop) { std::cout << "Epoll timeout" << std::endl; }