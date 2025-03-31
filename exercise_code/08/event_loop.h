#pragma once

#include "epoll.h"

class EventLoop {
private:
    Epoll* ep_;

public:
    EventLoop() : ep_(new Epoll) {}
    ~EventLoop() { delete ep_; }

    void Run();
    inline Epoll* GetEp() { return ep_; }
};