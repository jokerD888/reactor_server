#include "tcp_server.h"
TcpServer::TcpServer(const std::string& ip, const uint16_t& port) {
    acceptor_ = new Acceptor(&loop_, ip, port);
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
}
TcpServer::~TcpServer() { delete acceptor_; }
void TcpServer::Start() { loop_.Run(); }

void TcpServer::NewConnection(Socket* client_sock) { Connection* conn = new Connection(&loop_, client_sock); }