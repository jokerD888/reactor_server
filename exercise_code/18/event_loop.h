#pragma once

#include <functional>

#include "channel.h"
#include "epoll.h"

class Epoll;
class Channel;

class EventLoop {
private:
    Epoll* ep_;
    std::function<void(EventLoop*)> epoll_timeout_cb_;

public:
    EventLoop();
    ~EventLoop();

    void Run();
    inline Epoll* GetEp() { return ep_; }
    void UpdateChannel(Channel* ch);  // 把channel添加/更新到红黑树上
    inline void SetEpollTimeoutCb(std::function<void(EventLoop*)> cb) { epoll_timeout_cb_ = cb; }
};