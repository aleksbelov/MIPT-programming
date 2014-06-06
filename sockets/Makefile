
EXECUTABLE = server client
FLAGS = -std=c99 -pthread -lm

all : $(EXECUTABLE)

server : server.c
	gcc server.c sockets.h $(FLAGS) -o server

client : client.c
	gcc client.c sockets.h $(FLAGS) -o client

clean :
	rm $(EXECUTABLE)

.PHONY : all clean
