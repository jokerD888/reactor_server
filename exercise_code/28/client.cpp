#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./clinet <server_ip> <server_port>\n");
        printf("example: ./client 127.0.0.1 8080\n");
        return -1;
    }

    int sockfd;
    sockaddr_in servaddr;
    char buf[1024];
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket() failed\n");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect(%s:%s) failed\n", argv[1], argv[2]);
        return -1;
    }

    printf("connect ok\n");

    // for (int i = 0; i < 20000; ++i) {
    //     memset(buf, 0, sizeof(buf));
    //     printf("please input:");
    //     scanf("%s", buf);
    //     // 错误返回-1，对端已关闭连接返回0
    //     if (send(sockfd, buf, strlen(buf), 0) <= 0) {
    //         printf("write() failed\n");
    //         close(sockfd);
    //         return -1;
    //     }

    //     memset(buf, 0, sizeof(buf));
    //     // 对端正常关闭连接返回0
    //     // 若非阻塞模式（O_NONBLOCK），且当前无数据可读recv 返回 -1，并设置 errno = EAGAIN 或 EWOULDBLOCK（而非 0）。
    //     // 错误返回-1
    //     if (recv(sockfd, buf, sizeof(buf), 0) <= 0) {
    //         printf("read() failed\n");
    //         close(sockfd);
    //         return -1;
    //     }
    //     printf("recv: %s\n", buf);
    // }
    printf("begin() : %ld\n", time(NULL));

    for (int i = 0; i < 100000; ++i) {
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "hello world %d, today is nice, man!!!", i);
        char tmpbuf[1024];
        memset(tmpbuf, 0, sizeof(tmpbuf));
        int len = strlen(buf);
        memcpy(tmpbuf, &len, 4);
        memcpy(tmpbuf + 4, buf, len);
        if (send(sockfd, tmpbuf, 4 + len, 0) <= 0) {
            perror("send() failed\n");
            close(sockfd);
            return -1;
        }

        recv(sockfd, &len, 4, 0);
        if (len > 256) {
            printf("len = %d\n", len);
        }
        memset(buf, 0, sizeof(buf));
        recv(sockfd, buf, len, 0);
    }

    // for (int i = 0; i < 10000; ++i) {
    //     int len;
    //     recv(sockfd, &len, 4, 0);
    //     memset(buf, 0, sizeof(buf));
    //     recv(sockfd, buf, len, 0);
    //     // printf("recv: %s\n", buf);
    // }

    // printf当前时间

    // for (int i = 0; i < 10000; ++i) {
    //     memset(buf, 0, sizeof(buf));
    //     sprintf(buf, "hello world %d, today is nice, man!!!", i);
    //     // printf("send: %s\n", buf);
    //     if (send(sockfd, buf, strlen(buf), 0) <= 0) {
    //         perror("send() failed\n");
    //         close(sockfd);
    //         return -1;
    //     }
    //     memset(buf, 0, sizeof(buf));

    //     recv(sockfd, buf, 1024, 0);
    //     // printf("recv: %s\n", buf);
    //     //  sleep(1);
    // }

    printf("end() : %ld\n", time(NULL));
    return 0;
}