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

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("usage: ./tcpepoll ip port\n");
        printf("example: ./tcpepoll 127.0.0.1 8080\n");
        return -1;
    }

    // 创建监听套接字
    // AF_INET：使用 IPv4 地址族 -- SOCK_STREAM：表示面向连接的流式套接字（对应 TCP）。
    // IPPROTO_TCP：明确指定使用 TCP 协议（也可填 0，系统自动选择）。
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0) {
        perror("socket() failed");
        return -1;
    }

    // 通过 setsockopt 函数设置了多个套接字选项，旨在优化 TCP 服务器的行为和性能
    // setsockopt 第二个参数是套接字选项级别(SOL_SOCKET 和 IPPROTO_TCP)
    int opt = 1;
    // SO_REUSEADDR：允许地址和端口重用 -- TCP_NODELAY：禁用 Nagle 算法
    // SO_REUSEPORT：允许多进程/线程绑定同一端口 --SO_KEEPALIVE：启用 TCP 保活机制
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));  // 必须
    // TCP_NODELAY是唯一使用IPPROTO_TCP层的选项  !!!!!!!!!
    setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));  // 必须
    // 有用，但在reactor中意义不大
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));  // 可能有用

    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    // inet_addr：将字符串 IP 地址转换为网络字节序的二进制形式
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    // htons：将主机字节序的端口号转换为网络字节序
    servaddr.sin_port = htons(atoi(argv[2]));

    // sockaddr_in*强制转换为sockaddr*的原因与套接字接口的统一设计和​内存布局的兼容性有关
    // sockaddr是通用接口，sockaddr_in是IPV4专用
    if (bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind() failed");
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 128) != 0) {
        perror("listen() failed");
        close(listenfd);
        return -1;
    }

    // 创建一个 epoll 实例，返回其文件描述符（epollfd）,参数 size设为大于 0 的值即可。
    int epollfd = epoll_create(1);
    epoll_event ev;
    ev.data.fd = listenfd;  // 关联监听套接字
    ev.events = EPOLLIN;    // 监听可读事件,默认水平触发

    // 将监听套接字加入 epoll
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

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

                if (evs[i].data.fd == listenfd) {  // 监听套接字可读，表示有新连接请求
                    sockaddr_in cliaddr;
                    socklen_t clilen = sizeof(cliaddr);
                    int connfd = accept4(listenfd, (sockaddr*)&cliaddr, &clilen, SOCK_NONBLOCK);

                    printf("accept new connection from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

                    ev.data.fd = connfd;
                    ev.events = EPOLLIN | EPOLLET;  // 边缘触发
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
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