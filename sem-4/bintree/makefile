all : bt

bt : bt.o btlib.o
	gcc -Wall bt.o btlib.o -o bt

bt.o : bt.c 
	gcc -Wall -c bt.c -o bt.o

btlib.o : btlib.c
	gcc -Wall -c btlib.c -o btlib.o

.PHONY : clean

clean :
	rm -f *.o bt
