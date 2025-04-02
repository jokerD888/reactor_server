#include "channel.h"

void Channel::EnableReading() {
    events_ |= EPOLLIN;
    ep_->UpdateChannel(this);
}

void Channel::HandleEvent() {
    if (revents_ & EPOLLHUP) {  // EPOLLHUP：表示对端挂断（如客户端崩溃），需关闭套接字。
        printf("client(eventfd=%d) closed\n", fd_);
        close(fd_);
    } else if (revents_ & (EPOLLIN | EPOLLPRI)) {  // 接收缓冲区有数据可读
        // EPOLLPRI 表示有 ​紧急数据（Out-of-Band Data, OOB）​ 可读。
        // 触发条件：TCP 接收到带外数据（如 MSG_OOB 标志发送的数据）

        read_callback_();
    } else if (revents_ & EPOLLOUT) {  // EPOLLOUT 可写事件
        write_callback_();
    } else {
        printf("client(eventfd=%d) error.\n", fd_);
        close(fd_);
    }
}

void Channel::DisableReading() {
    events_ &= ~EPOLLIN;
    ep_->UpdateChannel(this);
}
void Channel::EnableWriting() {
    events_ |= EPOLLOUT;
    ep_->UpdateChannel(this);
}
void Channel::DisableWriting() {
    events_ &= ~EPOLLOUT;
    ep_->UpdateChannel(this);
}