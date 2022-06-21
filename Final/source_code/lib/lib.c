#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include<stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <inttypes.h>
#include <dirent.h>
#include <semaphore.h>
#include "lib.h"
#include<math.h>
#include <sys/socket.h> 
#include <netdb.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

void printMessage(char *message){
    while(((write(STDERR_FILENO, message, strlen(message))) == -1) && (errno == EINTR));
    free(message);
}

int openSocket(int socket1,char* IP,int PORT){
    struct sockaddr_in serverAddr;
    int client_fd;
    //Create socket file desc
    if ((socket1 = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("couldn't create socket !!");
        exit(EXIT_FAILURE);
    }
    serverAddr.sin_family = AF_INET;
    // serverAddr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(PORT);

    //Convert IPV4 or IPV6 addresses into binary from
    if (inet_pton(AF_INET,IP,&serverAddr.sin_addr) <= 0){
        perror("invalid addresss !!!");
        exit(EXIT_FAILURE);
    }
    client_fd = connect(socket1,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    if (client_fd == -1){
        perror("connection faild");
        exit(EXIT_FAILURE);
    }
    return socket1;// return the created socket.
}
