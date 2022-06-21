CC = gcc
CFLAGS = -ansi -Wall -Wshadow -std=gnu99 -lm -lpthread -lrt

makefile: ./Server/server.c ./Server/serverUtil.c ./Server/queue.c ./Client/client.c ./Client/clientUtil.c ./Servant/servantUtil.c ./lib/lib.c
	$(CC) -o server ./Server/server.c ./Server/serverUtil.c ./Server/queue.c ./lib/lib.c  $(CFLAGS)
	$(CC) -o client ./Client/client.c ./Client/clientUtil.c ./lib/lib.c $(CFLAGS)
	$(CC) -o servant ./Servant/servant.c ./Servant/servantUtil.c ./lib/lib.c $(CFLAGS)

clean:
	rm -rf *.o client server servant vg_logfile.out