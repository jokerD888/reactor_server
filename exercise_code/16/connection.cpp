#include "connection.h"

Connection::Connection(EventLoop* loop, Socket* client_sock) : loop_(loop), client_sock_(client_sock) {
    client_channel_ = new Channel(loop, client_sock_->fd());
    client_channel_->SetReadCallback(std::bind(&Channel::OnMessage, client_channel_));
    client_channel_->SetCloseCallback(std::bind(&Connection::CloseCallback, this));
    client_channel_->SetErrorCallback(std::bind(&Connection::ErrorCallback, this));
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