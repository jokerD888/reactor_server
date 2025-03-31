#include "tcp_server.h"
TcpServer::TcpServer(const std::string& ip, const uint16_t& port) {
    Socket* serv_sock = new Socket();
    InetAddress serv_addr(ip, port);
    serv_sock->SetKeepAlive();
    serv_sock->SetReuseAddr();
    serv_sock->SetReusePort();
    serv_sock->SetTcpNoDelay();
    serv_sock->Bind(serv_addr);
    serv_sock->Listen();

    Channel* server_channel = new Channel(loop_.GetEp(), serv_sock->fd());
    server_channel->SetReadCallback(std::bind(&Channel::NewConnection, server_channel, serv_sock));
    server_channel->EnableReading();
}
TcpServer::~TcpServer() {}
void TcpServer::Start() { loop_.Run(); }