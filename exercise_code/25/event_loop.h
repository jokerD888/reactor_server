#pragma once

#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#include "channel.h"
#include "epoll.h"

class Epoll;
class Channel;

class EventLoop {
private:
    std::unique_ptr<Epoll> ep_;
    std::function<void(EventLoop*)> epoll_timeout_cb_;
    pid_t thread_id_;                               // 事件循环所在线程id
    std::queue<std::function<void()>> task_queue_;  // 事件循环线程被eventfd唤醒后执行的任务队列
    std::mutex mutex_;                              // 互斥锁，保护task_queue_
    int wakeup_fd_;                                 // 用于唤醒事件循环线程的eventfd
    std::unique_ptr<Channel> wakeup_channel_;       // 用于唤醒事件循环线程的Channel
public:
    EventLoop();
    ~EventLoop();

    void Run();

    void UpdateChannel(Channel* ch);  // 把channel添加/更新到红黑树上
    void RemoveChannel(Channel* ch);
    inline void SetEpollTimeoutCb(std::function<void(EventLoop*)> cb) { epoll_timeout_cb_ = cb; }
    bool IsInLoopThread() const { return thread_id_ == syscall(SYS_gettid); }
    void QueueInLoop(std::function<void()> cb);  // 把任务添加到任务队列中

    void Wakeup();

    void HandleWakeup();
};