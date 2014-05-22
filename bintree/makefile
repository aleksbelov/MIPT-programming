# Compiler
CC=gcc
# Flags for the compiler
CFLAGS=-Wall -std=c99 -fprofile-arcs -ftest-coverage

all : bt

bt : bt.o btlib.o
	$(CC) $(CFLAGS) bt.o btlib.o -o bt

bt.o : bt.c btlib.h
	$(CC) $(CFLAGS) -c bt.c -o bt.o

btlib.o : btlib.c btlib.h
	$(CC) $(CFLAGS) -c btlib.c -o btlib.o

.PHONY : all clean

clean :
	rm -f *.o *.gcda *.gcov *.gcno bt
