#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "inet_address.h"
#include "socket.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("usage: ./tcpepoll ip port\n");
        printf("example: ./tcpepoll 127.0.0.1 8080\n");
        return -1;
    }
    Socket serv_sock;
    InetAddress serv_addr(argv[1], atoi(argv[2]));
    serv_sock.SetKeepAlive();
    serv_sock.SetReuseAddr();
    serv_sock.SetReusePort();
    serv_sock.SetTcpNoDelay();
    serv_sock.Bind(serv_addr);
    serv_sock.Listen();

    // 创建一个 epoll 实例，返回其文件描述符（epollfd）,参数 size设为大于 0 的值即可。
    int epollfd = epoll_create(1);
    epoll_event ev;
    ev.data.fd = serv_sock.fd();  // 关联监听套接字
    ev.events = EPOLLIN;          // 监听可读事件,默认水平触发

    // 将监听套接字加入 epoll
    epoll_ctl(epollfd, EPOLL_CTL_ADD, serv_sock.fd(), &ev);

    epoll_event evs[10];
    while (true) {
        // 10：最大返回事件数（避免一次性处理过多事件）。-1：无限等待，直到有事件触发。
        int infds = epoll_wait(epollfd, evs, 10, -1);
        if (infds < 0) {
            perror("epoll_wait() failed");
            break;
        }

        if (infds == 0) {
            perror("epoll_wait() timeout");
            continue;
        }

        for (int i = 0; i < infds; ++i) {
            if (evs[i].events & EPOLLHUP) {  // EPOLLHUP：表示对端挂断（如客户端崩溃），需关闭套接字。
                printf("client(eventfd=%d) closed\n", evs[i].data.fd);
                close(evs[i].data.fd);
            } else if (evs[i].events & (EPOLLIN | EPOLLPRI)) {  // 接收缓冲区有数据可读
                // EPOLLPRI 表示有 ​紧急数据（Out-of-Band Data, OOB）​ 可读。
                // 触发条件：TCP 接收到带外数据（如 MSG_OOB 标志发送的数据）

                if (evs[i].data.fd == serv_sock.fd()) {  // 监听套接字可读，表示有新连接请求

                    InetAddress client_addr;
                    Socket* client_sock = new Socket(serv_sock.Accept(client_addr));

                    printf("accept new connection from %s:%d\n", client_addr.ip(), client_addr.port());

                    ev.data.fd = client_sock->fd();
                    ev.events = EPOLLIN | EPOLLET;  // 边缘触发
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock->fd(), &ev);
                } else {
                    char buf[1024];
                    while (true) {  // 由于使用非阻塞IO，需要循环读取，直到没有数据可读
                        bzero(buf, sizeof(buf));
                        ssize_t nread = read(evs[i].data.fd, buf, sizeof(buf));
                        if (nread > 0) {
                            printf("recv(evfd=%d):%s\n", evs[i].data.fd, buf);
                            send(evs[i].data.fd, buf, nread, 0);
                        } else if (nread == -1 && errno == EINTR) {  //  读取数据的时候被中断，则继续读取
                            continue;
                        } else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                            // 没有数据可读，则跳出循环
                            // EAGAIN 和 EWOULDBLOCK 本质相同，表示非阻塞操作无法立即完成。
                            break;
                        } else if (nread == 0) {
                            printf("client(eventfd=%d) closed\n", evs[i].data.fd);
                            close(evs[i].data.fd);
                            break;
                        }
                    }
                }
            } else if (evs[i].events & EPOLLOUT) {  // EPOLLOUT 可写事件
            } else {
                printf("unknown event\n");
                close(evs[i].data.fd);
            }
        }
    }

    return 0;
}