#include "epoll.h"

Epoll::Epoll() {
    if ((epollfd_ = epoll_create(1)) == -1) {
        printf("epoll_create() failed(%d).\n", errno);
        exit(-1);
    }
}
Epoll::~Epoll() { close(epollfd_); }

std::vector<Channel*> Epoll::Loop(int timeout) {
    std::vector<Channel*> channels;
    bzero(&events_, sizeof(events_));
    // int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);
    int infds;
    do {
        infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);
    } while (infds == -1 && errno == EINTR);  // 如果是被信号中断，就重试

    if (infds == -1) {
        // 在reactor模型中，不建议使用信号，因为信号处理起来很麻烦，没有必要 ---陈硕
        perror("epoll_wait() failed.\n");
        exit(-1);
    }

    if (infds == 0) {
        return {};
    }

    for (int i = 0; i < infds; ++i) {
        Channel* ch = static_cast<Channel*>(events_[i].data.ptr);
        ch->SetRevents(events_[i].events);
        channels.push_back(ch);
    }
    return channels;
}

void Epoll::RemoveChannel(Channel* ch) {
    if (ch->InEpoll()) {
        // printf("RemoveChannel()\n");
        if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, ch->fd(), nullptr) == -1) {
            perror("epoll_ctl() failed.\n");
            exit(-1);
        }
    }
}
void Epoll::UpdateChannel(Channel* ch) {
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->GetEvents();
    if (ch->InEpoll()) {
        if ((epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev)) == -1) {
            perror("InEpoll epoll_ctl() failed");
            exit(-1);
        }

    } else {
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1) {
            perror("NoInEpoll epoll_ctl() failed");
            exit(-1);
        }
        ch->SetInEpoll();
    }
}