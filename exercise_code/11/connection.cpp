#include "connection.h"

Connection::Connection(EventLoop* loop, Socket* client_sock) : loop_(loop), client_sock_(client_sock) {
    client_channel_ = new Channel(loop, client_sock_->fd());
    client_channel_->SetReadCallback(std::bind(&Channel::OnMessage, client_channel_));
    client_channel_->SetInEpoll();
    client_channel_->SetEt();
    client_channel_->EnableReading();
}

Connection::~Connection() {
    delete client_sock_;
    delete client_channel_;
}
