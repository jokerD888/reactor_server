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

#include "epoll.h"
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

    Epoll ep;
    ep.AddFd(serv_sock.fd(), EPOLLIN);
    std::vector<epoll_event> evs(10);

    while (true) {
        evs = ep.Loop();

        for (auto& ev : evs) {
            if (ev.events & EPOLLHUP) {  // EPOLLHUP：表示对端挂断（如客户端崩溃），需关闭套接字。
                printf("client(eventfd=%d) closed\n", ev.data.fd);
                close(ev.data.fd);
            } else if (ev.events & (EPOLLIN | EPOLLPRI)) {  // 接收缓冲区有数据可读
                // EPOLLPRI 表示有 ​紧急数据（Out-of-Band Data, OOB）​ 可读。
                // 触发条件：TCP 接收到带外数据（如 MSG_OOB 标志发送的数据）

                if (ev.data.fd == serv_sock.fd()) {  // 监听套接字可读，表示有新连接请求

                    InetAddress client_addr;
                    Socket* client_sock = new Socket(serv_sock.Accept(client_addr));

                    printf("accept new connection from %s:%d\n", client_addr.ip(), client_addr.port());
                    ep.AddFd(client_sock->fd(), EPOLLIN | EPOLLET);
                } else {
                    char buf[1024];
                    while (true) {  // 由于使用非阻塞IO，需要循环读取，直到没有数据可读
                        bzero(buf, sizeof(buf));
                        ssize_t nread = read(ev.data.fd, buf, sizeof(buf));
                        if (nread > 0) {
                            printf("recv(evfd=%d):%s\n", ev.data.fd, buf);
                            send(ev.data.fd, buf, nread, 0);
                        } else if (nread == -1 && errno == EINTR) {  //  读取数据的时候被中断，则继续读取
                            continue;
                        } else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                            // 没有数据可读，则跳出循环
                            // EAGAIN 和 EWOULDBLOCK 本质相同，表示非阻塞操作无法立即完成。
                            break;
                        } else if (nread == 0) {
                            printf("client(eventfd=%d) closed\n", ev.data.fd);
                            close(ev.data.fd);
                            break;
                        }
                    }
                }
            } else if (ev.events & EPOLLOUT) {  // EPOLLOUT 可写事件
            } else {
                printf("unknown event\n");
                close(ev.data.fd);
            }
        }
    }

    return 0;
}