#pragma once

#include <sys/epoll.h>

#include "epoll.h"
#include "inet_address.h"
#include "socket.h"

class Epoll;

class Channel {
private:
    int fd_ = -1;  // channel对应的文件描述符,channel和fd之间是一对一
    // channel对应的epoll红黑树，channel和epoll之间是多对一的关系,一个channel只能属于一个epoll红黑树
    Epoll* ep_ = nullptr;
    // channel是否在epoll红黑树中,如果未添加，调用epoll_ctl()时用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD
    bool in_epoll_ = false;
    uint32_t events_ = 0;   // fd_需要监听的事件，listenfd和clientfd需要监听EPOLLIN，clientfd还可能需要监听EPOLLOUT
    uint32_t revents_ = 0;  // fd_已发生的事件
    bool is_listen_ = false;

public:
    Channel(Epoll* ep, int fd, bool is_listen = false) : fd_(fd), ep_(ep), is_listen_(is_listen) {}
    ~Channel() = default;

    inline int fd() { return fd_; }
    inline void SetEt() { events_ |= EPOLLET; }      // 设置边沿触发模式
    void EnableReading();                            // 让epoll_wait()监听fd_的读事件
    void SetInEpoll() { in_epoll_ = true; }          // 把in_epoll_ 设为true
    void SetRevents(uint32_t ev) { revents_ = ev; }  // 设置revents_
    bool InEpoll() { return in_epoll_; }             // 返回in_epoll_
    uint32_t GetEvents() { return events_; }         // 返回events_
    uint32_t GetRevents() { return revents_; }       // 返回revents_

    void HandleEvent(Socket* serv_sock);
};
