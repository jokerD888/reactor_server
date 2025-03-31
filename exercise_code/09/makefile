.PHONY:all clean
all:client tcpepoll

client:client.cpp
	g++ -o client client.cpp
tcpepoll:tcpepoll.cpp inet_address.cpp socket.cpp epoll.cpp channel.cpp event_loop.cpp tcp_server.cpp
	g++ -o tcpepoll tcpepoll.cpp inet_address.cpp socket.cpp epoll.cpp channel.cpp event_loop.cpp tcp_server.cpp

clean:
	rm -f client tcpepoll