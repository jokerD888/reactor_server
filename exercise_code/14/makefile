.PHONY:all clean
all:client tcpepoll

client:client.cpp
	g++ -g -o client client.cpp
tcpepoll:tcpepoll.cpp inet_address.cpp socket.cpp epoll.cpp channel.cpp event_loop.cpp tcp_server.cpp acceptor.cpp connection.cpp
	g++ -g -o tcpepoll tcpepoll.cpp inet_address.cpp socket.cpp epoll.cpp channel.cpp event_loop.cpp tcp_server.cpp acceptor.cpp connection.cpp

clean:
	rm -f client tcpepoll