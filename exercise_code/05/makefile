.PHONY:all clean
all:client tcpepoll

client:client.cpp
	g++ -o client client.cpp
tcpepoll:tcpepoll.cpp inet_address.cpp socket.cpp epoll.cpp
	g++ -o tcpepoll tcpepoll.cpp inet_address.cpp socket.cpp epoll.cpp

clean:
	rm -f client tcpepoll