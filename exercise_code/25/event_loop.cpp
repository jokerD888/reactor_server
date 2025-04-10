#include "event_loop.h"

EventLoop::EventLoop()
    : ep_(new Epoll()), wakeup_fd_(eventfd(0, EFD_NONBLOCK)), wakeup_channel_(new Channel(this, wakeup_fd_)) {
    wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleWakeup, this));
    wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {}

void EventLoop::Run() {
    thread_id_ = syscall(SYS_gettid);
    printf("EventLoop::Run() thread is %ld.\n", syscall(SYS_gettid));
    while (true) {
        std::vector<Channel*> channels = ep_->Loop(10 * 1000);
        if (channels.empty()) epoll_timeout_cb_(this);

        for (auto ch : channels) ch->HandleEvent();
    }
}
void EventLoop::UpdateChannel(Channel* ch) { ep_->UpdateChannel(ch); }

void EventLoop::RemoveChannel(Channel* ch) { ep_->RemoveChannel(ch); }

void EventLoop::QueueInLoop(std::function<void()> cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        task_queue_.push(cb);
    }

    // 唤醒事件循环线程
    Wakeup();
}

void EventLoop::Wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        printf("EventLoop::Wakeup() error\n");
    }
}

void EventLoop::HandleWakeup() {
    uint64_t val;
    ssize_t n = read(wakeup_fd_, &val, sizeof(val));
    if (n != sizeof(val)) {
        printf("EventLoop::HandleWakeup() error\n");
    }

    std::function<void()> task;
    std::lock_guard<std::mutex> lock(mutex_);
    while (!task_queue_.empty()) {
        task = std::move(task_queue_.front());
        task_queue_.pop();
        task();
    }
}