#include "tcp_server.h"
TcpServer::TcpServer(const std::string& ip, const uint16_t& port) {
    acceptor_ = new Acceptor(&loop_, ip, port);
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
}
TcpServer::~TcpServer() {
    delete acceptor_;
    for (auto& conn : conns_) delete conn.second;
}
void TcpServer::Start() { loop_.Run(); }

void TcpServer::NewConnection(Socket* client_sock) {
    Connection* conn = new Connection(&loop_, client_sock);
    printf("accept new connection from %s:%d\n", conn->ip().c_str(), conn->port());
    conns_[conn->fd()] = conn;
}