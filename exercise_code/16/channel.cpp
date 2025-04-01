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
    } else {
        printf("client(eventfd=%d) error.\n", fd_);
        close(fd_);
    }
}

void Channel::OnMessage() {
    char buf[1024];
    while (true) {  // 由于使用非阻塞IO，需要循环读取，直到没有数据可读
        bzero(buf, sizeof(buf));
        ssize_t nread = read(fd_, buf, sizeof(buf));
        if (nread > 0) {
            printf("recv(evfd=%d):%s\n", fd_, buf);
            send(fd_, buf, nread, 0);
        } else if (nread == -1 && errno == EINTR) {  //  读取数据的时候被中断，则继续读取
            continue;
        } else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // 没有数据可读，则跳出循环
            // EAGAIN 和 EWOULDBLOCK 本质相同，表示非阻塞操作无法立即完成。
            break;
        } else if (nread == 0) {
            printf("client(eventfd=%d) closed\n", fd_);
            close(fd_);
            break;
        }
    }
}