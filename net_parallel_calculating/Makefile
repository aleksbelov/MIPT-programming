.PHONY: all clean
all: client server

client: client.c
	gcc client.c -o client -g -Wall

server: server.c
	gcc server.c -o server -g -Wall -lpthread

clean:
	rm -f client server
