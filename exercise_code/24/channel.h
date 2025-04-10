#pragma once

#include <sys/epoll.h>

#include <functional>
#include <memory>

#include "epoll.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket.h"

class EventLoop;

class Channel {
private:
    int fd_ = -1;  // channel对应的文件描述符,channel和fd之间是一对一
    // channel对应的epoll红黑树，channel和epoll之间是多对一的关系,一个channel只能属于一个epoll红黑树
    EventLoop* loop_ = nullptr;
    // channel是否在epoll红黑树中,如果未添加，调用epoll_ctl()时用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD
    bool in_epoll_ = false;
    uint32_t events_ = 0;   // fd_需要监听的事件，listenfd和clientfd需要监听EPOLLIN，clientfd还可能需要监听EPOLLOUT
    uint32_t revents_ = 0;  // fd_已发生的事件

    std::function<void()> read_callback_;   // fd_读事件的回调函数,如果是acceptor,将回调Acceptor::NewConnection()
    std::function<void()> close_callback_;  // 关闭fd_的回调函数，将回调Connection::CloseCallback()
    std::function<void()> error_callback_;  // fd_发生错误的回调函数，将回调Connection::ErrorCallback()
    std::function<void()> write_callback_;  // fd_写事件的回调函数，将回调Connection::WriteCallback()

public:
    Channel(EventLoop* loop, int fd) : fd_(fd), loop_(loop) {}
    ~Channel() = default;

    inline int fd() { return fd_; }

    inline void SetEt() { events_ |= EPOLLET; }  // 设置边沿触发模式
    void EnableReading();                        // 让epoll_wait()监听fd_的读事件

    void DisableReading();                                  // 取消写事件
    void EnableWriting();                                   // 注册写事件
    void DisableWriting();                                  // 取消写事件
    void DisableAll();                                      // 取消所有事件
    void Remove();                                          // 从epoll红黑树中删除
    inline void SetInEpoll() { in_epoll_ = true; }          // 把in_epoll_ 设为true
    inline void SetRevents(uint32_t ev) { revents_ = ev; }  // 设置revents_

    inline bool InEpoll() { return in_epoll_; }        // 返回in_epoll_
    inline uint32_t GetEvents() { return events_; }    // 返回events_
    inline uint32_t GetRevents() { return revents_; }  // 返回revents_

    void HandleEvent();

    // 设置的回调函数即可能是accept新连接的回调函数，也可能是读消息的回调函数
    inline void SetReadCallback(std::function<void()> cb) { read_callback_ = cb; }
    inline void SetCloseCallback(std::function<void()> cb) { close_callback_ = cb; }
    inline void SetErrorCallback(std::function<void()> cb) { error_callback_ = cb; }
    inline void SetWriteCallback(std::function<void()> cb) { write_callback_ = cb; }
};
