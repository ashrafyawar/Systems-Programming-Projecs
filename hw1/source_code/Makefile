#bellow all command will compile all files 
CC=gcc

CFLAGS=-c -Wall

all: editorcompiled

editorcompiled: advancededit.o helperfunctions.o
	$(CC) advancededit.o helperfunctions.o -o editorcompiled

advancededit.o: advancededit.c
	$(CC) $(CFLAGS) advancededit.c


helperfunctions.o: helperfunctions.c
	$(CC) $(CFLAGS) helperfunctions.c

clean:
	rm -rf *.o editorcompiled