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
#include "clientUtil.h"
#include <sys/socket.h> 
#include <netdb.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

sig_atomic_t sigintcaught = 0;

pthread_t *threadsArray=NULL;
pthread_mutex_t mtx;
pthread_cond_t cond;
int arrived = 0,M = 0;
void freeThreadsPointersArr();
void siginthandler(int s);

void* myThread(void* clientRequestInd){
    
    struct clientReqServantInfo *threadClientReqStruct = (struct clientReqServantInfo*) clientRequestInd;
    int socketID = 0,size = 0;
    char* message;
    time_t t;time(&t);

    if (sigintcaught == 1){// if sigint has recieved.
        pthread_exit(NULL);
    }

    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Client-Thread-%d: Thread-%d has been created.\n",ctime(&t),threadClientReqStruct->threadInd,threadClientReqStruct->threadInd);
    printMessage(message); 

    // barrier part START
    pthread_mutex_lock(&mtx);
    ++arrived;
    while(arrived < M){pthread_cond_wait(&cond,&mtx);}
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mtx);
    // barrier part END
    
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Client-Thread-%d: I am requesting %s\n",ctime(&t),threadClientReqStruct->threadInd,threadClientReqStruct->requestStr);
    printMessage(message); 

    int data = 1;

    // SENT REQUEST
    socketID = openSocket(socketID,threadClientReqStruct->serverIP,threadClientReqStruct->serverPORT);
    threadClientReqStruct->flag = 1;
    send(socketID,threadClientReqStruct,sizeof(struct clientReqServantInfo),0);
    //********************************************************************************************
    // RECIEVE RESPONSE
    size = read(socketID,&data,sizeof(int));

    if (size == -1){
        perror("read failed");
        close(socketID);
        exit(EXIT_FAILURE);
    }
    //********************************************************************************************
    if (data == -1){// no such city
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Client-Thread-%d: The server’s response to %s is %d No Such City Found !!!\n",ctime(&t),threadClientReqStruct->threadInd,threadClientReqStruct->requestStr,data);
        printMessage(message);
    }
    else{
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Client-Thread-%d: The server’s response to %s is %d\n",ctime(&t),threadClientReqStruct->threadInd,threadClientReqStruct->requestStr,data);
        printMessage(message); 
    }


    if (sigintcaught == 1){// if sigint has recieved.
        pthread_exit(NULL);
    }

    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Client-Thread-%d: Terminating...\n",ctime(&t),threadClientReqStruct->threadInd);
    printMessage(message); 
    
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    
    setbuf(stdout, NULL);
    char *requestFile = NULL,*message,*IP = NULL,*eachReqToken,*eachReqTokenPt;
    struct clientReqServantInfo *clienRequestList;

    int requestFileFD = 0,PORT,s,requestCount = 0,indooo = 0;
    mode_t mode = MODE;
    struct sigaction newact;
    time_t t;time(&t);
    pthread_mutexattr_t mtxAttr;

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
    if (argc < 7){
        perror("No Sufficient Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./client -r requestFile -q PORT -s IP\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }else if (argc > 7){
        perror("Too Much Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./client -r requestFile -q PORT -s IP\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }

    //file paths read from terminal
    requestFile = argv[2];// requestfilepath
    PORT = atoi(argv[4]);// port
    IP = argv[6];// ip

    //input files settings:
    requestFileFD = open(requestFile,O_RDONLY,mode);
    if (requestFileFD == -1){// if file not found then print error on sdterr
        perror("Coudln't Open The Input File1 !!!\n");
        exit(EXIT_FAILURE);
    }

    //********************************************************************************************
    // LOAD DATA FROM REQUESTSFILE
    
    FILE* file = fopen(requestFile, "r"); /* should check the result */
    char line[256] = {'\0'};

    while (fgets(line, sizeof(line), file)) { 
       if (strcmp(line,"\n") == 0){
        }else{
        ++requestCount;    
        }

    }
    clienRequestList = (struct clientReqServantInfo*) malloc(sizeof(struct clientReqServantInfo) * requestCount);
    indooo = 0;fclose(file);
    file = fopen(requestFile, "r"); /* should check the result */

    while (fgets(line, sizeof(line), file)) {
        if (strcmp(line,"\n") == 0){
        }else{
            strcpy(clienRequestList[indooo].requestStr,line);
            clienRequestList[indooo].threadInd = indooo;
            ++indooo;   
        }
    }

    fclose(file);

    for (int i = 0; i < requestCount; ++i){
        eachReqTokenPt = malloc(sizeof(char) * MESSAGE_LEN);
        strcpy(eachReqTokenPt,clienRequestList[i].requestStr);
        eachReqToken = strtok(eachReqTokenPt, " ");
        // printf("%s\n",eachReqToken );
        if (eachReqToken != NULL){
            strcpy(clienRequestList[indooo].transactionCount,eachReqToken);
        }
        eachReqToken = strtok(NULL, " ");
        // printf("%s\n",eachReqToken );

        if (eachReqToken != NULL){
            strcpy(clienRequestList[indooo].transactionType,eachReqToken);
        }

        eachReqToken = strtok(NULL, "-");
        // printf("%s\n",eachReqToken );

        if (eachReqToken != NULL){
            clienRequestList[indooo].fromDayDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, "-");
        // printf("%s\n",eachReqToken );

        if (eachReqToken != NULL){
            clienRequestList[indooo].fromMonthDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, " ");
        // printf("%s\n",eachReqToken );

        if (eachReqToken != NULL){
            clienRequestList[indooo].fromYearDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, "-");
        // printf("%s\n",eachReqToken );

        if (eachReqToken != NULL){
            clienRequestList[indooo].toDayDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, "-");
        // printf("%s\n",eachReqToken );

        if (eachReqToken != NULL){
            clienRequestList[indooo].toMonthDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, " ");
        // printf("%s\n",eachReqToken );

        if (eachReqToken != NULL){
            clienRequestList[indooo].toYearDate = atoi(eachReqToken);
        }

        eachReqToken = strtok(NULL, " ");

        if (eachReqToken != NULL){
            strcpy(clienRequestList[indooo].cityName,eachReqToken);
        }else{
            strcpy(clienRequestList[indooo].transactionType,"empty");
        }      
        free(eachReqTokenPt);
    }
    M = requestCount;
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Client: I have loaded %d requests and I’m creating %d threads.\n",ctime(&t),M,M);
    printMessage(message);
    //********************************************************************************************

    // mutex initialization settings:
    s = pthread_mutexattr_init(&mtxAttr);
    if (s != 0){
        perror("pthread_mutexattr_init");
        close(requestFileFD);
        free(clienRequestList);
        exit(EXIT_FAILURE);
    }
    s = pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_NORMAL);
    if (s != 0){
        perror("pthread_mutexattr_settype");
        close(requestFileFD);
        free(clienRequestList);

        exit(EXIT_FAILURE);
    }
    s = pthread_mutex_init(&mtx, &mtxAttr);
    if (s != 0){
        perror("pthread_mutex_init");
        close(requestFileFD);
        free(clienRequestList);

        exit(EXIT_FAILURE);
    }
    s = pthread_mutexattr_destroy(&mtxAttr);
    if (s != 0){
        perror("pthread_mutexattr_destroy");
        close(requestFileFD);
        free(clienRequestList);

        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 
    
    // condition variable initialization settings:
    s = pthread_cond_init(&cond, NULL);
    if (s != 0){
        perror("pthread_cond_init");
        close(requestFileFD);
        free(clienRequestList);

        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 

    // threads creation settings:
    pthread_t threadsPointersList[M];
    for (int i = 0; i < M; ++i){
        strcpy(clienRequestList[i].serverIP,IP);
        clienRequestList[i].serverPORT = PORT;
        eachReqTokenPt = malloc(sizeof(char) * MESSAGE_LEN);
        strcpy(eachReqTokenPt,clienRequestList[i].requestStr);
        eachReqToken = strtok(eachReqTokenPt, " ");
        if (eachReqToken != NULL){
            strcpy(clienRequestList[i].transactionCount,eachReqToken);
        }
        eachReqToken = strtok(NULL, " ");
        if (eachReqToken != NULL){
            strcpy(clienRequestList[i].transactionType,eachReqToken);
        }

        eachReqToken = strtok(NULL, "-");
        if (eachReqToken != NULL){
            clienRequestList[i].fromDayDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, "-");
        if (eachReqToken != NULL){
            clienRequestList[i].fromMonthDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, " ");
        // printf("%s\n",eachReqToken );

        if (eachReqToken != NULL){
            clienRequestList[i].fromYearDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, "-");
        if (eachReqToken != NULL){
            clienRequestList[i].toDayDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, "-");
        if (eachReqToken != NULL){
            clienRequestList[i].toMonthDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, " ");
        if (eachReqToken != NULL){
            clienRequestList[i].toYearDate = atoi(eachReqToken);
        }
        eachReqToken = strtok(NULL, "\n");
        if (eachReqToken != NULL){
            strcpy(clienRequestList[i].cityName,eachReqToken);
        }else{
            strcpy(clienRequestList[i].cityName,"empty");
        }      
        s = pthread_create(&threadsPointersList[i],NULL, myThread,&clienRequestList[i]);
        if( s != 0){
            perror("pthread_create() error");
            close(requestFileFD);
            free(clienRequestList);

            exit(EXIT_FAILURE);
        }
        free(eachReqTokenPt);

    }
    threadsArray = threadsPointersList;
    
    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < M; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        close(requestFileFD);
        free(clienRequestList);

        freeThreadsPointersArr();
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 

    // wait for the thread to terminate.
    for (int i = 0; i < M; ++i){
        s = pthread_join(threadsPointersList[i],NULL);
        if (s != 0) {
            perror("pthread_join() error");
            close(requestFileFD);
            free(clienRequestList);

            freeThreadsPointersArr();
            exit(EXIT_FAILURE);
        }
    }
    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < M; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        close(requestFileFD);
        free(clienRequestList);

        freeThreadsPointersArr();
        exit(EXIT_FAILURE);
    }
    //********************************************************************************************


    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < M; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        close(requestFileFD);
        free(clienRequestList);

        freeThreadsPointersArr();
        exit(EXIT_FAILURE);
    }
    //********************************************************************************************
        
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Client: All threads have terminated, goodbye.\n",ctime(&t));
    printMessage(message);
    
    s = pthread_mutex_destroy(&mtx);
    if (s != 0){
        perror("pthread_mutex_destroy");
        close(requestFileFD);
        freeThreadsPointersArr();
        free(clienRequestList);

        exit(EXIT_FAILURE);
    }
    
    s = pthread_cond_destroy(&cond);
    if (s != 0){
        perror("pthread_cond_destroy");
        close(requestFileFD);
        freeThreadsPointersArr();
        free(clienRequestList);

        exit(EXIT_FAILURE);
    }

    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < M; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        close(requestFileFD);
        freeThreadsPointersArr();
        free(clienRequestList);

        exit(EXIT_FAILURE);
    }
    
    close(requestFileFD);
    free(clienRequestList);

    pthread_exit(0);
}


void freeThreadsPointersArr(){
    free(threadsArray);
}

void siginthandler(int s){
    int esaved = errno;
    if (s == SIGINT && sigintcaught != 1){
        sigintcaught = 1;
        for (int i = 0; i < M; i++){
            pthread_kill(threadsArray[i],SIGINT);
        }
    } 
    errno = esaved;
}
