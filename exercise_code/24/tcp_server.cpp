#include "tcp_server.h"

#include "echo_server.h"
TcpServer::TcpServer(const std::string& ip, const uint16_t& port, int thread_num) : thread_num_(thread_num) {
    main_loop_ = new EventLoop();
    main_loop_->SetEpollTimeoutCb(std::bind(&TcpServer::EpollTimeout, this, std::placeholders::_1));

    acceptor_ = new Acceptor(main_loop_, ip, port);
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));

    thread_pool_ = new ThreadPool(thread_num, "IO");
    // 创建从事件循环
    for (int i = 0; i < thread_num; i++) {
        sub_loops_.push_back(new EventLoop());
        sub_loops_[i]->SetEpollTimeoutCb(std::bind(&TcpServer::EpollTimeout, this, std::placeholders::_1));
        thread_pool_->AddTask(std::bind(&EventLoop::Run, sub_loops_[i]));
    }
}
TcpServer::~TcpServer() {
    delete acceptor_;
    delete main_loop_;

    for (auto& loop : sub_loops_) delete loop;
    delete thread_pool_;
}
void TcpServer::Start() { main_loop_->Run(); }

void TcpServer::NewConnection(std::unique_ptr<Socket> client_sock) {
    // 先获取 fd，再转移所有权!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int client_fd = client_sock->fd();  // 在移动前获取 fd
    std::shared_ptr<Connection> conn(new Connection(sub_loops_[client_fd % thread_num_], std::move(client_sock)));
    // std::shared_ptr<Connection> conn(
    //     new Connection(sub_loops_[client_sock->fd() % thread_num_], std::move(client_sock)));

    conn->SetCloseCallback(std::bind(&TcpServer::CloseConnection, this, std::placeholders::_1));
    conn->SetErrorCallback(std::bind(&TcpServer::ErrorConnection, this, std::placeholders::_1));
    conn->SetMessageCallback(std::bind(&TcpServer::Message, this, std::placeholders::_1, std::placeholders::_2));
    conn->SetSendCompleteCallback(std::bind(&TcpServer::SendComplete, this, std::placeholders::_1));
    // printf("accept new connection from %s:%d\n", conn->ip().c_str(), conn->port());
    conns_[conn->fd()] = conn;

    new_connection_cb_(conn);  // 连接完再回调
}

void TcpServer::CloseConnection(std::shared_ptr<Connection> conn) {
    close_connection_cb_(conn);  // 回调完再close
    // printf("client(eventfd=%d) closed\n", conn->fd());
    conns_.erase(conn->fd());
}
void TcpServer::ErrorConnection(std::shared_ptr<Connection> conn) {
    error_connection_cb_(conn);
    // printf("client(eventfd=%d) error.\n", conn->fd());
    conns_.erase(conn->fd());
}

void TcpServer::Message(std::shared_ptr<Connection> conn, std::string& message) { message_cb_(conn, message); }

void TcpServer::SendComplete(std::shared_ptr<Connection> conn) {
    // printf("send complete.\n");
    send_complete_cb_(conn);
}

void TcpServer::EpollTimeout(EventLoop* loop) {
    // printf("epoll timeout.\n");
    epoll_timeout_cb_(loop);
}