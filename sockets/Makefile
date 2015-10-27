
EXECUTABLE = server client
FLAGS = -std=c99 -pthread -lm -Wall

all : $(EXECUTABLE)

server : server.c sockets.h
	gcc server.c $(FLAGS) -o server

client : client.c sockets.h
	gcc client.c $(FLAGS) -o client

clean :
	rm $(EXECUTABLE)

.PHONY : all clean
