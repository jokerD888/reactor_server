
#include "tcp_server.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("usage: ./tcpepoll ip port\n");
        printf("example: ./tcpepoll 127.0.0.1 8080\n");
        return -1;
    }
    // tcp_server构造函数内new Acceptor(&loop_,ip,port)，acceptor_设置TcpServer::NewConnection回调函数
    // Acceptor构造函数内：socket()->bind()->listen(), new Channel(loop_,listen_fd),
    //      channel设置读事件的回调函数Acceptor::NewConnection
    // Acceptor::NewConnection这个回调函数是用来accept新连接的，
    // 而TcpServer::NewConnection是new一个Connection的，然后Connection的构造函数里面new
    //      一个Channel，这个Channel设置读事件的回调函数是Channel::OnMessage,用来读取数据
    // 可以看出，Acceptor和Connection的下层都是Channel
    // 其实上面这一连串，简单来说，创建listen_fd,为listen_fd设置读事件（新连接）的回调函数A,A里面有accept建立新的连接，之后调用acceptor内的回调函数new
    // Connection(其构造函数设置处理消息的回调函数Channel::OnMessage)
    // 我感觉封装的不好
    TcpServer tcp_server(argv[1], atoi(argv[2]));
    // TcpSever::Start()内调用EventLoop::Run(),而Run()内就是一个死循环，首先调用Epoll::Loop()获取所有新事件，然后遍历这些事件，调用对应的回调函数
    tcp_server.Start();

    return 0;
}