#include "tcp_server.h"

#include "echo_server.h"
TcpServer::TcpServer(const std::string& ip, const uint16_t& port, int thread_num)
    : thread_num_(thread_num),
      main_loop_(new EventLoop(true)),
      acceptor_(main_loop_.get(), ip, port),
      thread_pool_(thread_num, "IO") {
    // main_loop_ = new EventLoop();
    main_loop_->SetEpollTimeoutCb(std::bind(&TcpServer::EpollTimeout, this, std::placeholders::_1));

    acceptor_.SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));

    // 创建从事件循环
    for (int i = 0; i < thread_num; i++) {
        sub_loops_.emplace_back(new EventLoop(false, 5, 60));
        sub_loops_[i]->SetEpollTimeoutCb(std::bind(&TcpServer::EpollTimeout, this, std::placeholders::_1));
        sub_loops_[i]->SetTimerCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
        thread_pool_.AddTask(std::bind(&EventLoop::Run, sub_loops_[i].get()));
    }
}
TcpServer::~TcpServer() {}
void TcpServer::Start() { main_loop_->Run(); }

void TcpServer::NewConnection(std::unique_ptr<Socket> client_sock) {
    // 先获取 fd，再转移所有权!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int client_fd = client_sock->fd();  // 在移动前获取 fd
    std::shared_ptr<Connection> conn(new Connection(sub_loops_[client_fd % thread_num_].get(), std::move(client_sock)));
    // std::shared_ptr<Connection> conn(
    //     new Connection(sub_loops_[client_sock->fd() % thread_num_], std::move(client_sock)));

    conn->SetCloseCallback(std::bind(&TcpServer::CloseConnection, this, std::placeholders::_1));
    conn->SetErrorCallback(std::bind(&TcpServer::ErrorConnection, this, std::placeholders::_1));
    conn->SetMessageCallback(std::bind(&TcpServer::Message, this, std::placeholders::_1, std::placeholders::_2));
    conn->SetSendCompleteCallback(std::bind(&TcpServer::SendComplete, this, std::placeholders::_1));
    // printf("accept new connection from %s:%d\n", conn->ip().c_str(), conn->port());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conns_[conn->fd()] = conn;
    }

    sub_loops_[client_fd % thread_num_]->NewConnection(conn);
    // 打印conn智能指针计数

    if (new_connection_cb_) new_connection_cb_(conn);  // 连接完再回调
}

void TcpServer::CloseConnection(std::shared_ptr<Connection> conn) {
    close_connection_cb_(conn);  // 回调完再close
    // printf("client(eventfd=%d) closed\n", conn->fd());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conns_.erase(conn->fd());
    }
}
void TcpServer::ErrorConnection(std::shared_ptr<Connection> conn) {
    error_connection_cb_(conn);
    // printf("client(eventfd=%d) error.\n", conn->fd());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conns_.erase(conn->fd());
    }
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

void TcpServer::RemoveConnection(int fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (conns_.find(fd) == conns_.end()) {  // 可能客户端在超时之前就已经主动断开连接了
        printf("fd = %d not found\n", fd);
        return;
    }

    conns_.erase(fd);
}
