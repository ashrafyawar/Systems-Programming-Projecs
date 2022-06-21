all: hw4

hw4: threads.o
	gcc -o hw4 threads.o -pthread -lrt

threads.o: threads.c
	gcc -c threads.c -Wall -Wextra -pedantic -g  -I .

clean:
	rm -rf *.o hw4 vg_logfile.out