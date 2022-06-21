#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <error.h>
#include <fcntl.h>
#include "become_daemon.h"
#include <signal.h>

volatile sig_atomic_t sigterm = 0;
volatile sig_atomic_t sighub = 0;

volatile sig_atomic_t sigIntInterrupt = 0;
   
void sigterm_handler(int signo){
	if (signo == SIGTERM)
        sigterm = 1;
}

void sighub_handler(int signo){
	if (signo == SIGHUP)
        sighub = 1;
}

void sigint_handler(int signo) {
    if (signo == SIGINT)
        sigIntInterrupt = 1;
}

void become_daemon(){
	int maxfd, fd;
	struct sigaction sa;

	switch(fork()){
		case -1	: exit(EXIT_FAILURE);
		case  0	: break;
		default	: exit(EXIT_SUCCESS);
	}

	if(setsid() == -1){
		perror("setsid()");
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sighub_handler;
	sigaction(SIGHUP, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigterm_handler;
	sigaction(SIGTERM, &sa, NULL);

	switch(fork()){
		case -1	: exit(EXIT_FAILURE);
		case  0	: break;
		default	: exit(EXIT_SUCCESS);
	}

	umask(0);

	fd = ((maxfd = sysconf(_SC_OPEN_MAX)) > 0) ? maxfd-1 : BD_MAX_CLOSE-1;
	for( ; fd>=0 ; --fd) {
		close(fd);
	}

	close(STDIN_FILENO);

	fd = open("/dev/null/", O_RDWR);	


	// if(fd != STDIN_FILENO) exit(EXIT_FAILURE);

	// if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) exit(EXIT_FAILURE);

	// if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) exit(EXIT_FAILURE);
}