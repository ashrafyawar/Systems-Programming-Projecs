all: hw3named hw3unnamed

hw3named: named.o
	gcc -o hw3named named.o -pthread -lrt

hw3unnamed: unnamed.o
	gcc -o hw3unnamed unnamed.o -pthread -lrt

named.o: named.c
	gcc -c named.c -Wall -Wextra -pedantic -g  -I .

unnamed.o: unnamed.c
	gcc -c unnamed.c -Wall -Wextra -pedantic -g  -I .

clean:
	rm -rf *.o hw3named hw3unnamed named unnamed vg_logfile.out