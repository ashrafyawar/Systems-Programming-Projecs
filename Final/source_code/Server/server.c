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
#include<math.h>
#include "serverUtil.h"
#include <sys/socket.h> 
#include <netdb.h> 
#include <time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include "queue.h"

pthread_mutex_t mutex_barrier= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_queue = PTHREAD_COND_INITIALIZER;

int size_queue = 0;
int arrived=0;
int available=0;
int serverSock;
Queue* queue = NULL;
struct servantInfoStruct servantInfoList[MESSAGE_LEN];

int servantCount = 0;
char* IP = "127.0.0.1";
ARGUMENT argument;
sig_atomic_t sigintcaught = 0;

void siginthandler(int s);
void freeServants();

void* myThread(void* arg){
    time_t t;time(&t);
    int my_id = *((int*)arg);
    my_id += my_id;
    char* message = NULL;
    if(pthread_mutex_lock(&mutex_barrier) != 0){
        perror("error locking mutex");
        exit(EXIT_FAILURE);
    }
    ++arrived;
    ++available;

    if(arrived < argument.Number_of_Threads){
        if(pthread_cond_wait(&cond,&mutex_barrier) != 0){
        perror("error condition wait");
        exit(EXIT_FAILURE);
    }  
    }
    else{
        if(pthread_cond_broadcast(&cond) != 0){
            perror("error condition broadcast");
            exit(EXIT_FAILURE);
        } 
    }

    if(pthread_mutex_unlock(&mutex_barrier) != 0){
        perror("error mutex unlocking");
        exit(EXIT_FAILURE);
    }


    int fd_server = 0;
    int data;

    while(1){
        
        if (sigintcaught == 1){
            perror("Server revieved SIGINT, Exiting...\n");
            free(arg);
            freeServants();
            return(NULL);   
        }
        
        pthread_mutex_lock(&mutex_queue);
        
        while(size_queue == 0){ pthread_cond_wait(&cond_queue, &mutex_queue);}
        
        fd_server = poll_int(&queue);
        --size_queue;

        pthread_mutex_unlock(&mutex_queue);
        struct clientReqServantInfo responseStruct;
        if(read(fd_server,&responseStruct, sizeof(struct clientReqServantInfo)) == -1){
            perror("error occures reading from client to server!!");
            freeServants();
            exit(EXIT_FAILURE);
        }
        if (sigintcaught == 1){
            printf("SING INT RECIVED IN SERVER...\n");
            exit(EXIT_FAILURE);
        }
        if (responseStruct.flag == 1){ // if client sent request
            int localSockFd = 0,CurrServantPort,cityFound = 0;
            int cmpRes = strcmp(responseStruct.cityName,"empty");
            if (cmpRes == 0){// if city name is empty
                int totalTran = 0;
                for (int i = 0; i < servantCount; ++i){

                    localSockFd = openSocket(localSockFd,IP,servantInfoList[i].port);
                    send(localSockFd,&responseStruct,sizeof(struct clientReqServantInfo),0);

                    if (sigintcaught == 1){
                        printf("SING INT RECIVED IN SERVER...\n");
                        exit(EXIT_FAILURE);
                    }

                    // RECIEVE RESPONSE
                    int size = read(localSockFd,&data,sizeof(int));

                    if (size == -1){
                        perror("read failed");
                        close(localSockFd);
                        exit(EXIT_FAILURE);
                    }
                    totalTran += data;
                    close(localSockFd);
                }
                if(write(fd_server,&totalTran, sizeof(int)) == -1){
                    perror("error occures reading from client to server!!");
                    exit(EXIT_FAILURE);
                }

            // SENT REQUEST
            }else{ // if city name is not empty

                for (int i = 0; i < servantCount; ++i){
                    if (strstr(servantInfoList[i].cities,responseStruct.cityName) != NULL) {// found the servant !!!
                        cityFound = 1;
                        CurrServantPort = servantInfoList[i].port;
                    }
                }
                
                if (cityFound == 1){// if city exist
                  
                    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
                    sprintf(message,"[%.19s] Request Arrived\n",ctime(&t));
                    printMessage(message); 
                    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
                    sprintf(message,"[%.19s] Contacting Servent %d \n",ctime(&t),CurrServantPort);
                    printMessage(message); 
                    localSockFd = openSocket(localSockFd,IP,CurrServantPort);
                    send(localSockFd,&responseStruct,sizeof(struct clientReqServantInfo),0);

                    // RECIEVE RESPONSE

                    int size = read(localSockFd,&data,sizeof(int));

                    if (size == -1){
                        perror("read failed");
                        close(localSockFd);
                        exit(EXIT_FAILURE);
                    }

                    
                    if(write(fd_server,&data, sizeof(int)) == -1){
                        perror("error occures reading from client to server!!");
                        exit(EXIT_FAILURE);
                    }
                    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
                    sprintf(message,"[%.19s] Response Arrived: %d, Forwarded to Client \n",ctime(&t),data);
                    printMessage(message); 
                    
                    close(localSockFd);
                }else{// no such city
                    data = -1;
                    // localSockFd = openSocket(localSockFd,IP,CurrServantPort);

                    // if(write(fd_server,&data, sizeof(int)) == -1){
                    //     perror("error occures reading from client to server!!");
                    //     exit(EXIT_FAILURE);
                    // }
                }
                cityFound = 0;
            }

        }else if (responseStruct.flag == 2){ // if servant sent port and city into.
            servantInfoList[servantCount].port = responseStruct.servantPort;
            servantInfoList[servantCount].pid = responseStruct.pid;

            strcpy(servantInfoList[servantCount].cities,responseStruct.servantAvailableCities);
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"[%.19s] Servant %d present at port %d handaling cities %s-%s\n",ctime(&t),servantInfoList[servantCount].pid,
            servantInfoList[servantCount].port,responseStruct.startCity,responseStruct.endCity);
            printMessage(message); 

            ++servantCount;
        }
        // parse the recived string 

        pthread_mutex_lock(&mutex_barrier);
        available++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex_barrier);
    }
    freeServants();
    free(arg);
    return(NULL);
}
void freeServants();
int main(int argc, char const *argv[]){

	int PORT,threadNum,socketFd;
	char *message;
	time_t t;time(&t);
    struct sockaddr_in server_addres; 
    int s,reqSocketFD;
    int len_of_addr = sizeof(struct sockaddr_in);
    struct sigaction newact;
    
    newact.sa_handler = &siginthandler; /* set the new handler */
    newact.sa_flags = 0;
    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT,&newact, NULL) == -1)){
        perror("Failed to install SIGINT signal handler");
        exit(EXIT_FAILURE);
    }

    if (sigintcaught == 1){// if sigint has recieved.
        exit(EXIT_FAILURE);
    }

    // check if the user has entered sufficient arguments.
    if (argc < 5){
        perror("No Sufficient Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./server -p PORT -t numberOfThreads\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }else if (argc > 5){
        perror("Too Much Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./server -p PORT -t numberOfThreads\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }

    //file paths read from terminal

    PORT = atoi(argv[2]);
    threadNum = atoi(argv[4]);

    if (threadNum < 5){
        perror("Very less thread count !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./server -p PORT -t numberOfThreads (more than 5 threads)\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }

    //******************************************************************************************** 

    // threads creation settings:
    pthread_t threadsPointersList[threadNum];
    for (int i = 0; i < threadNum; ++i){
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        s = pthread_create(&threadsPointersList[i],NULL, myThread,arg);
        if( s != 0){
            perror("pthread_create() error");
            exit(EXIT_FAILURE);
        }
    }    
    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < threadNum; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 


// SERVER SOCKET SETTINGS START **************************************************************************************************

    /* Socket create */
    socketFd = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFd == -1) { 
        perror(" socket creation failed.\n"); 
        exit(EXIT_FAILURE); 
    } 
    bzero(&server_addres, sizeof(server_addres)); 
  
    /* assigning ip and port */ 
    server_addres.sin_family = AF_INET; 
    server_addres.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addres.sin_port = htons(PORT); 

    /* Binds the created socket to given ip and port */
    if ((bind(socketFd, (struct sockaddr*)&server_addres, sizeof(server_addres))) <  0) { 
        perror(" server: socket bind failed.\n"); 
        exit(EXIT_FAILURE);
    } 
  
    /* Server listens */
    if ((listen(socketFd, 4096)) < 0) { 
        perror(" server listen failed.\n"); 
        exit(EXIT_FAILURE); 
    }
// SERVER SOCKET SETTINGS END **************************************************************************************************

    while(1){ //  main thread revices the requests and adds it into queue.
        if (sigintcaught == 1){
            printf("SING INT RECIVED IN SERVER...\n");
            exit(EXIT_FAILURE);
        }
        reqSocketFD = accept(socketFd,(struct sockaddr*)&server_addres,(socklen_t *)&len_of_addr);
        if (reqSocketFD < 0){
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        // add fd to the queuelist
        pthread_mutex_lock(&mutex_queue);
        offer_int(&queue, reqSocketFD);
        size_queue++;
        pthread_cond_broadcast(&cond_queue);
        pthread_mutex_unlock(&mutex_queue);

        if (sigintcaught == 1){
            printf("SING INT RECIVED IN SERVER...\n");
            exit(EXIT_FAILURE);
        }
    }
  
    // wait for the thread to terminate.
    for (int i = 0; i < threadNum; ++i){
        s = pthread_join(threadsPointersList[i],NULL);
        if (s != 0) {
            perror("pthread_join() error");
            exit(EXIT_FAILURE);
        }
    }
    close(socketFd);
	return 0;
}

void siginthandler(int s){
    int esaved = errno;
    sigintcaught = 1;
    for (int i = 0; i < servantCount; i++){
        kill(servantInfoList[i].pid,SIGINT);
    }
     
    errno = esaved;
}
void freeServants(){
    // free(servantInfoList);
}
