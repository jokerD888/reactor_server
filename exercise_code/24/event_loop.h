#pragma once

#include <sys/syscall.h>
#include <unistd.h>

#include <functional>
#include <memory>

#include "channel.h"
#include "epoll.h"

class Epoll;
class Channel;

class EventLoop {
private:
    std::unique_ptr<Epoll> ep_;
    std::function<void(EventLoop*)> epoll_timeout_cb_;

public:
    EventLoop();
    ~EventLoop();

    void Run();

    void UpdateChannel(Channel* ch);  // 把channel添加/更新到红黑树上
    void RemoveChannel(Channel* ch);
    inline void SetEpollTimeoutCb(std::function<void(EventLoop*)> cb) { epoll_timeout_cb_ = cb; }
};