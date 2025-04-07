#include "event_loop.h"

EventLoop::EventLoop() : ep_(new Epoll()) {}

EventLoop::~EventLoop() { delete ep_; }

void EventLoop::Run() {
    printf("EventLoop::Run() thread is %ld.\n", syscall(SYS_gettid));
    while (true) {
        std::vector<Channel*> channels = ep_->Loop(10 * 1000);
        if (channels.empty()) epoll_timeout_cb_(this);

        for (auto ch : channels) ch->HandleEvent();
    }
}
void EventLoop::UpdateChannel(Channel* ch) { ep_->UpdateChannel(ch); }

void EventLoop::RemoveChannel(Channel* ch) { ep_->RemoveChannel(ch); }