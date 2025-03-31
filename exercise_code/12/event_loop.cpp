#include "event_loop.h"

EventLoop::EventLoop() : ep_(new Epoll()) {}

EventLoop::~EventLoop() { delete ep_; }

void EventLoop::Run() {
    while (true) {
        std::vector<Channel*> channels = ep_->Loop();
        for (auto ch : channels) ch->HandleEvent();
    }
}
void EventLoop::UpdateChannel(Channel* ch) {
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->GetEvents();
    if (ch->InEpoll()) {
        if ((epoll_ctl(ep_->GetEpollFd(), EPOLL_CTL_MOD, ch->fd(), &ev)) == -1) {
            perror("InEpoll epoll_ctl() failed");
            exit(-1);
        }

    } else {
        if (epoll_ctl(ep_->GetEpollFd(), EPOLL_CTL_ADD, ch->fd(), &ev) == -1) {
            perror("NoInEpoll epoll_ctl() failed");
            exit(-1);
        }
        ch->SetInEpoll();
    }
}