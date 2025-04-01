#pragma once

#include "channel.h"
#include "epoll.h"

class Epoll;
class Channel;

class EventLoop {
private:
    Epoll* ep_;

public:
    EventLoop();
    ~EventLoop();

    void Run();
    inline Epoll* GetEp() { return ep_; }
    void UpdateChannel(Channel* ch);  // 把channel添加/更新到红黑树上
};