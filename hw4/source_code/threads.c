#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include<stdlib.h>
#include <signal.h>
#include <sys/sem.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define MESSAGE_LEN 1000
#define BUFFER_LIMIT 1// buffer size used to read from file.
#define ACCESS_PERMISSION_FLAG_INPUT  O_RDONLY
#define MODE S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH


sig_atomic_t sigintcaught = 0;
union semun{
    int val;
    struct semid_ds * buf;
    unsigned short* array;
    struct seminfo* __buf;
}sem_attr;
int C = 0,N = 0,semid;
pthread_t *consumerThreadsArray=NULL;

void printMessage(char *message);
void siginthandler(int s){
    int esaved = errno;
    if (s == SIGINT && sigintcaught != 1){
        sigintcaught = 1;
        for (int i = 0; i < C; i++){
            pthread_kill(consumerThreadsArray[i],SIGINT);
        }
    } 
    errno = esaved;
}

void *supplierThread(void *ptr){
    
    char buf[BUFFER_LIMIT],*message;
    int fd = *((int*) ptr), bytesread;
    time_t t;time(&t);

    for (;;){

        while((bytesread = read(fd,buf,BUFFER_LIMIT)) == -1 && errno == EINTR);// read bytes from the input file
        if (bytesread <= 0 ){// if the end of file is reached.
            break;
        }
        if (buf[0] == '1'){// supplier has read 1 from the file
            
            struct sembuf semOp;
            semOp.sem_num = 0;
            semOp.sem_op = 1;
            semOp.sem_flg = 0;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"[%.19s] Supplier: read from input a '%s'. Current amounts: %d x '1', %d x '2'\n",ctime(&t),buf,semctl(semid,0,GETVAL),semctl(semid,1,GETVAL));
            printMessage(message);
            
            if (semop(semid, &semOp, 1) == -1){// perfrom operation on semaphore set
                perror("semop on semid");
                exit(EXIT_FAILURE);
            }
        }else if (buf[0] == '2'){// supplier has read 2 from the file

            struct sembuf semOp;
            semOp.sem_num = 1;
            semOp.sem_op = 1;
            semOp.sem_flg = 0;
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"[%.19s] Supplier: read from input a '%s'. Current amounts: %d x '1', %d x '2'\n",ctime(&t),buf,semctl(semid,0,GETVAL),semctl(semid,1,GETVAL));
            printMessage(message);

            if (semop(semid, &semOp, 1) == -1){// perfrom operation on semaphore set
                perror("semop on semid");
                exit(EXIT_FAILURE);
            }
        }
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Supplier: delivered a '%s'. Post-delivery amounts: %d x '1', %d x '2'\n",ctime(&t),buf,semctl(semid,0,GETVAL),semctl(semid,1,GETVAL));
        printMessage(message);

    }
    
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] The Supplier has left.\n",ctime(&t));
    printMessage(message);
    
    free(ptr);
    return(NULL);
}

void *consumerThread(void *ptr){

    int ptrLocal = *((int *) ptr);
    char *message;
    time_t t;time(&t);
        
    struct sembuf semOp[2];
    semOp[0].sem_num = 0;
    semOp[0].sem_op = -1;
    semOp[0].sem_flg = 0;

    semOp[1].sem_num = 1;
    semOp[1].sem_op = -1;
    semOp[1].sem_flg = 0;

    for (int i = 0; i < N; ++i){
        
        
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Consumer-%d at iteration %d (waiting). Current amounts: %d x '1', %d x '2'\n",ctime(&t),ptrLocal,i,semctl(semid,0,GETVAL),semctl(semid,1,GETVAL));
        printMessage(message);
        
        if (semop(semid, semOp, 2) == -1){// perfrom operation on semaphore set
            perror("semop on semid");
            exit(EXIT_FAILURE);
        }
                
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Consumer-%d at iteration %d (consumed). Post-consumption amounts: %d x '1', %d x '2'\n",ctime(&t),ptrLocal,i,semctl(semid,0,GETVAL),semctl(semid,1,GETVAL));
        printMessage(message);

    }
    
    free(ptr);

    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Consumer-%d has left\n",ctime(&t),ptrLocal);
    printMessage(message);
   
    return(NULL);
}

int main(int argc, char **argv){
    setbuf(stdout, NULL);
    char *input_path = NULL,*message;
    int inpfd = 0,s;
    mode_t mode = MODE;
    pthread_t supplierThreadPointer;
    pthread_attr_t attr;
    struct sigaction newact;
    time_t t;time(&t);
    
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
        sprintf(message,"[%.19s] Usage: vg ./hw4 -C 10 -N 5 -F inputfilePath\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }else if (argc > 7){
        perror("Too Much Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./hw4 -C 10 -N 5 -F inputfilePath\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }

    //file paths read from terminal
    C = atoi(argv[2]);
    N = atoi(argv[4]);
    input_path = argv[6];// input file path
    if (C < 4 && N < 1){
        perror("error:");
        exit(EXIT_FAILURE);   
    }
    //input file settings:
    inpfd = open(input_path,ACCESS_PERMISSION_FLAG_INPUT,mode);
    if (inpfd == -1){// if file not found then print error on sdterr
        perror("Coudln't Open The Input File !!!\n");
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 

    //system V semaphore settings:

    semid = semget(/*semaphore key*/ IPC_PRIVATE,/*semaphore count*/ 2,/*semaphore permissions*/  0777 | IPC_CREAT);
    if (semid == -1){
        perror("semid semget() error");
        exit(EXIT_FAILURE);
    }

    sem_attr.val = 0;
    if (semctl(/*semaphore set id*/ semid,/*semaphore set element index*/ 0,/*semaphore operations*/ SETVAL,/*semaphore union object*/ sem_attr) == -1){
        perror("semctl()");
        exit(EXIT_FAILURE);
    }
    if (semctl(/*semaphore set id*/ semid,/*semaphore set element index*/ 1,/*semaphore operations*/ SETVAL,/*semaphore union object*/ sem_attr) == -1){
        perror("semctl()");
        exit(EXIT_FAILURE);
    }

    if (sigintcaught == 1){// if sigint has recieved.
        if (semctl(/*semaphore set id*/ semid,/*semaphore set element index*/ 0,/*semaphore operations*/ IPC_RMID,sem_attr) == -1){
            perror("semctl() destroy semaphore");
            exit(EXIT_FAILURE);
        }
        close(inpfd);
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 
    
    // create supplier thread:
    s = pthread_attr_init(&attr);
    if (s != 0){
        perror("pthread_attr_init\n");
        exit(EXIT_FAILURE);
    }
    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (s != 0){
        perror("pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
    }
    
    int *arg = malloc(sizeof(*arg));
    if ( arg == NULL ) {
        perror("Couldn't allocate memory for arg.\n");
        exit(EXIT_FAILURE);
    }
    *arg = inpfd;
    s = pthread_create( &supplierThreadPointer, &attr, supplierThread,arg);
    if( s != 0){
        perror("pthread_create() error");
        exit(EXIT_FAILURE);
    }
    s = pthread_attr_destroy(&attr);
    if (s != 0){
        perror("pthread_attr_destroy");
        exit(EXIT_FAILURE);
    }

    if (sigintcaught == 1){// if sigint has recieved.
        if (semctl(/*semaphore set id*/ semid,/*semaphore set element index*/ 0,/*semaphore operations*/ IPC_RMID,sem_attr) == -1){
            perror("semctl() destroy semaphore");
            exit(EXIT_FAILURE);
        }
        close(inpfd);
        exit(EXIT_FAILURE);
    }
    //*********************************************************************************************

    // create consumer threads:
    pthread_t consumerThreadsPointersList[C];
    for (int i = 0; i < C; ++i){

        int *arg = malloc(sizeof(*arg));
        if ( arg == NULL ) {
            perror("Couldn't allocate memory for arg.\n");
            exit(EXIT_FAILURE);
        }
        *arg = i;
        s = pthread_create(&consumerThreadsPointersList[i],NULL, consumerThread,arg);
        if( s != 0){
            perror("pthread_create() error");
            exit(EXIT_FAILURE);
        }
    }
    consumerThreadsArray = consumerThreadsPointersList;
    if (sigintcaught == 1){// if sigint has recieved.
        if (semctl(/*semaphore set id*/ semid,/*semaphore set element index*/ 0,/*semaphore operations*/ IPC_RMID) == -1){
            perror("semctl() destroy semaphore");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < C; ++i){        
            pthread_cancel(consumerThreadsPointersList[i]);
        }
        close(inpfd);
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 

    // wait for the threads to terminate:
    for (int i = 0; i < C; ++i){        
        s = pthread_join(consumerThreadsPointersList[i],NULL);
        if (s != 0) {
            perror("pthread_join() error");
            exit(EXIT_FAILURE);
        }
    }
    //******************************************************************************************** 

    close(inpfd);    
    if (semctl(/*semaphore set id*/ semid,/*semaphore set element index*/ 0,/*semaphore operations*/ IPC_RMID) == -1){
        perror("semctl() destroy semaphore");
        exit(EXIT_FAILURE);
    }

    pthread_exit(0);
}

void printMessage(char *message){
    while(((write(STDERR_FILENO, message, strlen(message))) == -1) && (errno == EINTR));
    free(message);
}