all: hw5

hw5: threads_synchronization.o
	gcc -o hw5 threads_synchronization.o -pthread -lrt -lm

threads_synchronization.o: threads_synchronization.c
	gcc -c threads_synchronization.c -Wall -Wextra -pedantic -g  -I .

clean:
	rm -rf *.o hw5 outputFile.csv vg_logfile.out