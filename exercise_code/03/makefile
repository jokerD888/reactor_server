.PHONY:all clean
all:client tcpepoll

client:client.cpp
	g++ -o client client.cpp
tcpepoll:tcpepoll.cpp InetAddress.cpp
	g++ -o tcpepoll tcpepoll.cpp InetAddress.cpp

clean:
	rm -f client tcpepoll