#include "epoll.h"

Epoll::Epoll() {
    if ((epollfd_ = epoll_create(1)) == -1) {
        printf("epoll_create() failed(%d).\n", errno);
        exit(-1);
    }
}
Epoll::~Epoll() { close(epollfd_); }

void Epoll::UpdateChannel(Channel* ch) {
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->GetEvents();
    if (ch->InEpoll()) {
        if ((epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev)) == -1) {
            perror("epoll_ctl() failed.");
            exit(-1);
        }

    } else {
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1) {
            perror("epoll_ctl() failed.\n");
            exit(-1);
        }
        ch->SetInEpoll();
    }
}
std::vector<Channel*> Epoll::Loop(int timeout) {
    std::vector<Channel*> channels;
    bzero(&events_, sizeof(events_));
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
        Channel* ch = static_cast<Channel*>(events_[i].data.ptr);
        ch->SetRevents(events_[i].events);
        channels.push_back(ch);
    }
    return channels;
}