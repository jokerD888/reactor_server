#include "connection.h"

Connection::Connection(EventLoop* loop, Socket* client_sock) : loop_(loop), client_sock_(client_sock) {
    client_channel_ = new Channel(loop, client_sock_->fd());
    client_channel_->SetReadCallback(std::bind(&Connection::OnMessage, this));
    client_channel_->SetCloseCallback(std::bind(&Connection::CloseCallback, this));
    client_channel_->SetErrorCallback(std::bind(&Connection::ErrorCallback, this));
    client_channel_->SetWriteCallback(std::bind(&Connection::WriteCallback, this));
    client_channel_->SetEt();
    client_channel_->EnableReading();
}

Connection::~Connection() {
    delete client_sock_;
    delete client_channel_;
}

int Connection::fd() const { return client_sock_->fd(); }
std::string Connection::ip() const { return client_sock_->ip(); }
uint16_t Connection::port() const { return client_sock_->port(); }

void Connection::CloseCallback() { close_cb_(this); }
void Connection::ErrorCallback() { error_cb_(this); }

void Connection::WriteCallback() {
    // 尝试把 output_buffer_ 中的所有数据发送出去
    int nsend = send(fd(), output_buffer_.data(), output_buffer_.size(), 0);
    if (nsend > 0) output_buffer_.Erase(0, nsend);
    // 如果发送缓冲区中没有数据了，不再关注写事件
    if (output_buffer_.size() == 0) client_channel_->DisableWriting();
}

void Connection::OnMessage() {
    char buf[1024];
    while (true) {  // 由于使用非阻塞IO，需要循环读取，直到没有数据可读
        bzero(buf, sizeof(buf));
        ssize_t nread = read(fd(), buf, sizeof(buf));
        if (nread > 0) {
            input_buffer_.Append(buf, nread);
        } else if (nread == -1 && errno == EINTR) {  //  读取数据的时候被中断，则继续读取
            continue;
        } else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // 没有数据可读，则跳出循环
            // EAGAIN 和 EWOULDBLOCK 本质相同，表示非阻塞操作无法立即完成。
            // printf("recv(eventfd=%d):%s\n", fd(), input_buffer_.data());
            while (true) {
                int len;
                memcpy(&len, input_buffer_.data(), 4);
                if (input_buffer_.size() < len + 4) break;

                std::string message(input_buffer_.data() + 4, len);
                input_buffer_.Erase(0, len + 4);

                printf("message (eventfd=%d):%s\n", fd(), message.c_str());
                message_cb_(this, message);
            }
            break;
        } else if (nread == 0) {  // 客户端连接已断开
            CloseCallback();
            break;
        }
    }
}

void Connection::Send(const char* data, size_t size) {
    output_buffer_.Append(data, size);
    // 注册写事件
    client_channel_->EnableWriting();
}
