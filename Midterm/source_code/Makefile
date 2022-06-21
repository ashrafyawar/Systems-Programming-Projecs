#bellow all command will compile all files 
CC=gcc

CFLAGS=-c -Wall
# >>>>>>>
all: serverY serverZ client
# >>>>>>>

# >>>>>>>
serverY: serverY.o
	$(CC) serverY.o -o serverY -lm

serverZ: serverZ.o
	$(CC) serverZ.o -o serverZ -lm

client: client.o
	$(CC) client.o -o client -lm
# >>>>>>>
serverY.o: serverY.c
	$(CC) $(CFLAGS) serverY.c -lm

serverZ.o: serverZ.c
	$(CC) $(CFLAGS) serverZ.c -lm

client.o: client.c
	$(CC) $(CFLAGS) client.c -lm

# >>>>>>>
clean:
	rm -rf *.o response_* requests yToZFifoPath serverY serverZ client vg_logfile.out