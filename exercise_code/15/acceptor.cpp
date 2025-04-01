#include "acceptor.h"

#include "connection.h"

Acceptor::Acceptor(EventLoop *loop, const std::string &ip, uint16_t port) : loop_(loop) {
    serv_sock_ = new Socket();
    InetAddress serv_addr(ip, port);
    serv_sock_->SetKeepAlive();
    serv_sock_->SetReuseAddr();
    serv_sock_->SetReusePort();
    serv_sock_->SetTcpNoDelay();
    serv_sock_->Bind(serv_addr);
    serv_sock_->Listen();

    accept_channel_ = new Channel(loop_, serv_sock_->fd());
    accept_channel_->SetReadCallback(std::bind(&Acceptor::NewConnection, this));
    accept_channel_->EnableReading();
}
Acceptor::~Acceptor() {
    delete accept_channel_;
    delete serv_sock_;
}

void Acceptor::NewConnection() {
    InetAddress client_addr;
    Socket *client_sock = new Socket(serv_sock_->Accept(client_addr));

    new_connection_callback_(client_sock);
}
void Acceptor::SetNewConnectionCallback(std::function<void(Socket *)> cb) { new_connection_callback_ = cb; }