#bellow all command will compile all files 
CC=gcc

CFLAGS=-c -Wall

all: hw02compiled childcompiled

hw02compiled: parent.o child.o helperfunctions.o
	$(CC) parent.o helperfunctions.o -o hw02compiled -lm

childcompiled: child.o
	$(CC) child.o -o childcompiled -lm

parent.o: parent.c
	$(CC) $(CFLAGS) parent.c

child.o: child.c
	$(CC) $(CFLAGS) child.c

helperfunctions.o: helperfunctions.c
	$(CC) $(CFLAGS) helperfunctions.c

clean:
	rm -rf *.o hw02compiled childcompiled vg_logfile.out