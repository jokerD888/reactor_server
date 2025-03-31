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
    Channel* server_channel = new Channel(&ep, serv_sock.fd());
    server_channel->SetReadCallback(std::bind(&Channel::NewConnection, server_channel, &serv_sock));
    server_channel->EnableReading();

    while (true) {
        std::vector<Channel*> channels = ep.Loop();

        for (auto& ch : channels) {
            ch->HandleEvent();
        }
    }

    return 0;
}