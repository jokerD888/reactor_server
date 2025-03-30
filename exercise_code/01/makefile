.PHONY:all clean
all:client tcpepoll

client:client.cpp
	g++ -o client client.cpp
tcpepoll:tcpepoll.cpp
	g++ -o tcpepoll tcpepoll.cpp

clean:
	rm -f client tcpepoll