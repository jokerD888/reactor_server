
#include "tcp_server.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("usage: ./tcpepoll ip port\n");
        printf("example: ./tcpepoll 127.0.0.1 8080\n");
        return -1;
    }
    TcpServer tcp_server(argv[1], atoi(argv[2]));
    tcp_server.Start();

    return 0;
}