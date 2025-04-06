#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <vector>

#include "channel.h"

class Channel;

class Epoll {
private:
    static constexpr int MaxEvents = 128;
    int epollfd_ = -1;
    epoll_event events_[MaxEvents];

public:
    Epoll();
    ~Epoll();
    // void AddFd(int fd, uint32_t op);
    // void UpdateChannel(Channel* ch);  // 把channel添加/更新到红黑树上
    std::vector<Channel*> Loop(int timeout = -1);
    int GetEpollFd() const { return epollfd_; }
};