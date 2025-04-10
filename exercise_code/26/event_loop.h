#pragma once

#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>

#include "channel.h"
#include "connection.h"
#include "epoll.h"

class Epoll;
class Channel;
class Connection;

class EventLoop {
private:
    std::unique_ptr<Epoll> ep_;
    std::function<void(EventLoop*)> epoll_timeout_cb_;
    pid_t thread_id_;                                   // 事件循环所在线程id
    std::queue<std::function<void()>> task_queue_;      // 事件循环线程被eventfd唤醒后执行的任务队列
    std::mutex mutex_;                                  // 互斥锁，保护task_queue_
    std::mutex mutex_conns_;                            // 互斥锁，保护conns_
    int wakeup_fd_;                                     // 用于唤醒事件循环线程的eventfd
    std::unique_ptr<Channel> wakeup_channel_;           // 用于唤醒事件循环线程的Channel
    int timer_fd_;                                      // 用于定时器的timerfd
    std::unique_ptr<Channel> timer_channel_;            // 用于定时器的Channel
    bool is_main_loop_;                                 // 是否是主事件循环
    std::map<int, std::shared_ptr<Connection>> conns_;  // 存放所有的connection对象

    std::function<void(int)> timer_callback_;  // 定时器到期后执行的回调函数

    int time_interval_;  // 定时器的时间间隔，单位是秒
    int time_out_;       // 定时器的超时时间，单位是秒

    // 1.在事件循环中添加map<int,std::unique_ptr<Channel>>，存放运行在该事件循环上的全部connection对象
    // 2.如果闹钟时间到了，遍历conns_,判断每个connection对象的闹钟时间是否到了
    // 3.如果超时了，从conns_中删除该对象
    // 4.还需要从TcpServer.conn_中删除connection对象
public:
    EventLoop(bool main_loop, int time_interval = 30,
              int time_out = 60);  // 构造函数，创建epoll对象和wakeup_channel_对象
    ~EventLoop();

    void Run();

    void UpdateChannel(Channel* ch);  // 把channel添加/更新到红黑树上
    void RemoveChannel(Channel* ch);
    inline void SetEpollTimeoutCb(std::function<void(EventLoop*)> cb) { epoll_timeout_cb_ = cb; }
    bool IsInLoopThread() const { return thread_id_ == syscall(SYS_gettid); }
    void QueueInLoop(std::function<void()> cb);  // 把任务添加到任务队列中

    void Wakeup();

    void HandleWakeup();

    void HandleTimerfd();

    void NewConnection(std::shared_ptr<Connection> conn);  // 把Connection对象保存到conn_中

    void SetTimerCallback(std::function<void(int)> cb) { timer_callback_ = cb; }  // 设置定时器到期后的回调函数
};