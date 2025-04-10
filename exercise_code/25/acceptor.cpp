#include "acceptor.h"

#include "connection.h"

Acceptor::Acceptor(EventLoop *loop, const std::string &ip, uint16_t port)
    : loop_(loop), serv_sock_(), accept_channel_(loop_, serv_sock_.fd()) {
    InetAddress serv_addr(ip, port);
    serv_sock_.SetKeepAlive();
    serv_sock_.SetReuseAddr();
    serv_sock_.SetReusePort();
    serv_sock_.SetTcpNoDelay();
    serv_sock_.Bind(serv_addr);
    serv_sock_.Listen();

    accept_channel_.SetReadCallback(std::bind(&Acceptor::NewConnection, this));
    accept_channel_.EnableReading();
}
Acceptor::~Acceptor() {}

void Acceptor::NewConnection() {
    InetAddress client_addr;
    std::unique_ptr<Socket> client_sock(new Socket(serv_sock_.Accept(client_addr)));
    client_sock->SetIpPort(client_addr.ip(), client_addr.port());
    new_connection_callback_(std::move(client_sock));
}
void Acceptor::SetNewConnectionCallback(std::function<void(std::unique_ptr<Socket>)> cb) {
    new_connection_callback_ = cb;
}