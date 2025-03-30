#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <vector>

class Epoll {
private:
    static constexpr int MaxEvents = 128;
    int epollfd_ = -1;
    epoll_event events_[MaxEvents];

public:
    Epoll();
    ~Epoll();
    void AddFd(int fd, uint32_t op);
    std::vector<epoll_event> Loop(int timeout = -1);
};