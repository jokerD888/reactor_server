#include "epoll.h"

Epoll::Epoll() {
    if ((epollfd_ = epoll_create(1)) == -1) {
        printf("epoll_create() failed(%d).\n", errno);
        exit(-1);
    }
}
Epoll::~Epoll() { close(epollfd_); }
void Epoll::AddFd(int fd, uint32_t op) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = op;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        printf("epoll_ctl() failed(%d).\n", errno);
        exit(-1);
    }
}
std::vector<epoll_event> Epoll::Loop(int timeout) {
    std::vector<epoll_event> evs;
    bzero(&evs, sizeof(evs));
    int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);

    if (infds == -1) {
        perror("epoll_wait() failed.\n");
        exit(-1);
    }

    if (infds == 0) {
        printf("epoll_wait() timeout.\n");
        return {};
    }

    for (int i = 0; i < infds; ++i) {
        evs.push_back(events_[i]);
    }
    return evs;
}