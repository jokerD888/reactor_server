.PHONY:all clean
all:client tcpepoll

client:client.cpp
	g++ -g -o client client.cpp
tcpepoll:tcpepoll.cpp inet_address.cpp socket.cpp epoll.cpp channel.cpp event_loop.cpp tcp_server.cpp acceptor.cpp connection.cpp buffer.cpp echo_server.cpp thread_pool.cpp
	g++ -g -o tcpepoll tcpepoll.cpp inet_address.cpp socket.cpp epoll.cpp channel.cpp event_loop.cpp tcp_server.cpp acceptor.cpp connection.cpp buffer.cpp echo_server.cpp thread_pool.cpp

clean:
	rm -f client tcpepolll