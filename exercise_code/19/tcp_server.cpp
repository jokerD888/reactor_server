#include "tcp_server.h"

#include "echo_server.h"
TcpServer::TcpServer(const std::string& ip, const uint16_t& port) {
    acceptor_ = new Acceptor(&loop_, ip, port);
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
    loop_.SetEpollTimeoutCb(std::bind(&TcpServer::EpollTimeout, this, std::placeholders::_1));
}
TcpServer::~TcpServer() {
    delete acceptor_;
    for (auto& conn : conns_) delete conn.second;
}
void TcpServer::Start() { loop_.Run(); }

void TcpServer::NewConnection(Socket* client_sock) {
    Connection* conn = new Connection(&loop_, client_sock);
    conn->SetCloseCallback(std::bind(&TcpServer::CloseConnection, this, std::placeholders::_1));
    conn->SetErrorCallback(std::bind(&TcpServer::ErrorConnection, this, std::placeholders::_1));
    conn->SetMessageCallback(std::bind(&TcpServer::Message, this, std::placeholders::_1, std::placeholders::_2));
    conn->SetSendCompleteCallback(std::bind(&TcpServer::SendComplete, this, std::placeholders::_1));
    // printf("accept new connection from %s:%d\n", conn->ip().c_str(), conn->port());
    conns_[conn->fd()] = conn;

    new_connection_cb_(conn);  // 连接完再回调
}

void TcpServer::CloseConnection(Connection* conn) {
    close_connection_cb_(conn);  // 回调完再close
    // printf("client(eventfd=%d) closed\n", conn->fd());
    conns_.erase(conn->fd());
    delete conn;
}
void TcpServer::ErrorConnection(Connection* conn) {
    error_connection_cb_(conn);
    // printf("client(eventfd=%d) error.\n", conn->fd());
    conns_.erase(conn->fd());
    delete conn;
}

void TcpServer::Message(Connection* conn, std::string message) { message_cb_(conn, message); }

void TcpServer::SendComplete(Connection* conn) {
    // printf("send complete.\n");
    send_complete_cb_(conn);
}

void TcpServer::EpollTimeout(EventLoop* loop) {
    // printf("epoll timeout.\n");
    epoll_timeout_cb_(loop);
}