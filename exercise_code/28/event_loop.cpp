#include "event_loop.h"

int CreateTimerfd(int sec = 30) {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    struct itimerspec timeout;
    memset(&timeout, 0, sizeof(timeout));
    timeout.it_value.tv_sec = 5;
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timer_fd, 0, &timeout, NULL);
    return timer_fd;
}

EventLoop::EventLoop(bool main_loop, int time_interval, int time_out)
    : ep_(new Epoll()),
      wakeup_fd_(eventfd(0, EFD_NONBLOCK)),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      timer_fd_(CreateTimerfd(time_interval)),
      timer_channel_(new Channel(this, timer_fd_)),
      is_main_loop_(main_loop),
      time_interval_(time_interval),
      time_out_(time_out),
      stop_(false) {
    wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleWakeup, this));
    wakeup_channel_->EnableReading();
    timer_channel_->SetReadCallback(std::bind(&EventLoop::HandleTimerfd, this));
    timer_channel_->EnableReading();
}

EventLoop::~EventLoop() {}

void EventLoop::Run() {
    thread_id_ = syscall(SYS_gettid);
    // printf("EventLoop::Run() thread is %ld.\n", syscall(SYS_gettid));
    while (!stop_) {
        std::vector<Channel*> channels = ep_->Loop(10 * 1000);
        if (channels.empty()) epoll_timeout_cb_(this);

        for (auto ch : channels) ch->HandleEvent();
    }
}
void EventLoop ::Stop() {
    stop_ = true;
    Wakeup();
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

void EventLoop::HandleTimerfd() {
    // 重新计数
    struct itimerspec timeout;
    memset(&timeout, 0, sizeof(timeout));
    timeout.it_value.tv_sec = time_interval_;
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timer_fd_, 0, &timeout, NULL);

    if (is_main_loop_) {
    } else {
        // printf("EventLoop::HandleTimerfd() thread is %ld. fd = ", syscall(SYS_gettid));
        time_t now = time(0);
        for (auto& conn : conns_) {
            // printf("%d ", conn.first);
            if (conn.second->IsTimeout(now, time_out_)) {
                {
                    std::lock_guard<std::mutex> lock(mutex_conns_);
                    conns_.erase(conn.first);
                }
                timer_callback_(conn.first);
            }
        }
        // printf("\n");
    }
}

void EventLoop::NewConnection(std::shared_ptr<Connection> conn) {
    std::lock_guard<std::mutex> lock(mutex_conns_);
    conns_[conn->fd()] = conn;
}