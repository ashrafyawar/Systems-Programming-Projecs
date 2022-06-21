#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include<stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include "become_daemon.h"

#define LOCK_MODES F_WRLCK
#define CLIENT_FIFO_TEMPLATE "response_%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)
#define BUFFER_LIMIT 300000

volatile sig_atomic_t sigintcaught = 0; // used as hanlder indicator
volatile sig_atomic_t sigterm = 0;
volatile sig_atomic_t sighub = 0;

struct request{
    pid_t pid;
    int matrixLen;
    char payLoad[BUFFER_LIMIT];
};
struct response{
    int isInvertable;
};
   
void sigterm_handler(int signo){
    int esaved = errno;
    if (signo == SIGTERM){
        sigterm = 1;
    }
    errno = esaved;
}

void sighub_handler(int signo){
    int esaved = errno;
    if (signo == SIGHUP){
        sighub = 1;
    }
    errno = esaved;
}

void siginthandler(){
    int esaved = errno;
    sigintcaught = 1;
    errno = esaved;
}

void getCofactor(int* mat, int* temp, int p, int q, int n){
    int i = 0, j = 0;
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            if (row != p && col != q) {
                int t = *((mat+row*n))+col;
                 *(((temp+i*n))+j++) = t;
                if (j == n - 1) {
                    j = 0;
                    i++;
                }
            }
        }
    }
}

int determinantOfMatrix(int* mat, int n){
    int D = 0; // Initialize result
    if (n == 1){
        return *((mat+0))+0;
    }
    int temp[n][n]; // To store cofactors
    int sign = 1; // To store sign multiplier
    // Iterate for each element of first row
    for (int f = 0; f < n; f++) {
        getCofactor((int *)mat,(int *) temp, 0, f, n);
        D += sign * (*((mat+0))+f) * determinantOfMatrix((int*)temp, n - 1);
        sign = -sign;
    }
    return D;
}

int main(int argc, char const *argv[]){
    
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    const char* serverFifoPath = NULL;
    const char* logFilePath = NULL;
    const char* yToZFifoPath= "yToZFifoPath";
    char* printBufferPointer = (char*) malloc(1000 * sizeof(char));
    int poolSize1 = 0, poolSize2 = 0,serverFd = 0, dummyFd = 0, clientFd = 0,logFileFd = 0,sleepDuration = 0,printBufferPointerLen = 1000;
    int handeledInvReqs = 0, handeledNotInvReqs = 0,forwardedRequest = 0,forkVal = 0,yToZFifoFd = 0;
    int serverZArgArrSize = 4,workerPID = 0;
    struct request req;
    struct response resp;
    struct sigaction newact;
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    time_t t;   // not a primitive datatype
    time(&t);
    char** arr = (char**)malloc(serverZArgArrSize * sizeof(char*)); // malloc used to store output file descriptor to send it to child.
    for (int i = 0; i < serverZArgArrSize; i++){
        arr[i] = (char*)malloc(BUFFER_LIMIT * sizeof(char));
    }

    newact.sa_handler = &siginthandler; /* set the new handler */
    newact.sa_flags = 0;
    sigaction(SIGINT, &newact, NULL);

// Deamon Part START >>>>>>>>>>>>>>>>>>>>>>>>>>>
    // int maxfd, fd,flags = 1;
    // struct sigaction sa;
    
    // switch(fork()){
    //     case -1 : exit(EXIT_FAILURE);
    //     case  0 : break;
    //     default : exit(EXIT_SUCCESS);
    // }

    // if(setsid() == -1){
    //     perror("setsid()");
    //     _exit(EXIT_FAILURE);
    // }

    // memset(&sa, 0, sizeof(sa));
    // sa.sa_handler = sighub_handler;
    // sigaction(SIGHUP, &sa, NULL);

    // memset(&sa, 0, sizeof(sa));
    // sa.sa_handler = sigterm_handler;
    // sigaction(SIGTERM, &sa, NULL);

    // switch(fork()){
    //     case -1 : exit(EXIT_FAILURE);
    //     case  0 : break;
    //     default : exit(EXIT_SUCCESS);
    // }

    // if (!(flags & BD_NO_UMASK0)){
    //     umask(0);
    // }
    // if (!(flags & BD_NO_CHDIR)){
    //     chdir("/");
    // }

    // if (!(flags & BD_NO_CLOSE_FILES)) { /* Close all open files */
    //     maxfd = sysconf(_SC_OPEN_MAX);
    //     if (maxfd == -1){
    //         maxfd = BD_MAX_CLOSE;
    //     }
    //     /* so take a guess */
    //     for (fd = 0; fd < maxfd; fd++){
    //         close(fd);
    //     }
    // }

    // if (!(flags & BD_NO_REOPEN_STD_FDS)) {
    //     close(STDIN_FILENO);
    //     /* Reopen standard fd's to /dev/null */
    //     fd = open("/dev/null", O_RDWR);
    //     if (fd != STDIN_FILENO){
    //         return -1;
    //     }
    //     if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO){
    //         return -1;
    //     }
    //     if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO){
    //         return -1;
    //     }
    // }
    
// Deamon Part END >>>>>>>>>>>>>>>>>>>>>>>>>>>
    
    if (sigintcaught == 1){// sig int
        printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
        sprintf(printBufferPointer,"[%.19s] SIGINT recieved, terminating Z and exiting server Y. Total requests handeled: %d, %d invertable, %d not. %d requests were forwared.\n",ctime(&t),handeledInvReqs + handeledNotInvReqs,handeledInvReqs,handeledNotInvReqs,forwardedRequest);
        
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        
        if (write(logFileFd,printBufferPointer,strlen(printBufferPointer)) == -1){
            perror("failed to write into log file");
            free(printBufferPointer);
        }
        
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);
        //UNLOCK ***********************************************
        
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr);   
        free(printBufferPointer);
        return 1;
    }
 
   // check if the user has entered sufficient arguments.
    if (argc < 11){
        perror("No Sufficient Parameters To Execute The Edit !!!\n");
        fprintf(stdout,"Usage: vg ./serverY -s pathToServerFifo -o pathToLogFile –p poolSize -r poolSize2 -t 2\n");
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr); 
        return 1;
    }else if (argc >11){
        perror("Too Much Parameters To Execute The Edit !!!\n");
        fprintf(stdout,"Usage: vg ./serverY -s pathToServerFifo -o pathToLogFile –p poolSize -r poolSize2 -t 2\n");
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr); 
        return 1;

    }
    
    //file paths read from terminal
    serverFifoPath = argv[2];
    logFilePath = argv[4];
    poolSize1 = atoi(argv[6]);
    poolSize2 = atoi(argv[8]);
    sleepDuration = atoi(argv[10]);
    logFileFd = open(logFilePath,O_WRONLY| O_TRUNC,0644);

    if (logFileFd == -1){
        perror("logFileFd:");
    }

    if (mkfifo(yToZFifoPath, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){// make fifo of the server y to server z
        
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        
        if (write(logFileFd,"make fifo:\n",strlen("make fifo:\n")) == -1){
            perror("failed to write into log file");
        }
        
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);
        //UNLOCK ***********************************************

        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr);
        close(logFileFd);       
        _exit(EXIT_FAILURE);
    }

    // serverZ args
    strcpy(arr[0],yToZFifoPath);
    strcpy(arr[1],logFilePath);
    sprintf(arr[2],"%d",poolSize2);
    sprintf(arr[3],"%d",sleepDuration);

    forkVal = fork();
    
    // instantiate serverZ
    switch(forkVal){
    case -1:
        perror("Forking Failed");
    case 0: //  child process area
        execv("./serverZ",arr);
        _exit(EXIT_SUCCESS);
    default:
        break;
    }

    sprintf(printBufferPointer,"[%.19s] Server Y (%s, poolSize = %d, sleepDuration = %d) Started\n Instantiated Server Z",ctime(&t),logFilePath,poolSize1,sleepDuration);
    // LOCK*************************************************
    fcntl(logFileFd,F_SETLKW,&lock);
    // LOCK*************************************************
    if (write(logFileFd,printBufferPointer,strlen(printBufferPointer)) == -1){
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
        //UNLOCK ***********************************************
        perror("failed to write into log file");
        free(printBufferPointer);
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr);
        close(logFileFd); 
        _exit(EXIT_FAILURE);
    }
    //UNLOCK ***********************************************
    lock.l_type = F_UNLCK;    
    fcntl(logFileFd,F_SETLKW,&lock);
    //UNLOCK ***********************************************
    free(printBufferPointer);

    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT,&newact, NULL) == -1)){
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************

        if (write(logFileFd,"Failed to install SIGINT signal handler\n",strlen("Failed to install SIGINT signal handler\n")) == -1){
            perror("failed to write into log file");
        }
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
        //UNLOCK ***********************************************
        
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr); 
        _exit(EXIT_FAILURE);
    }
    
    // pid_t workersPidsArr[poolSize1]; // workers pids array
    int workerPipeArr[poolSize1][2]; // workers pipes array      
    
    umask(0); // set mask to 0
    if (mkfifo(serverFifoPath, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){// make fifo of the server
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        if (write(logFileFd,"make fifo:\n",strlen("make fifo:\n")) == -1){
            perror("failed to write into log file");
        }
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
        //UNLOCK *********************************************** 
        
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr);
        close(logFileFd);       
        _exit(EXIT_FAILURE);
    }

    serverFd = open(serverFifoPath, O_RDONLY);// open fifo of the server
    if (serverFd == -1){
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        if (write(logFileFd,"serverFifoPath:\n",strlen("serverFifoPath:\n")) == -1){
            perror("failed to write into log file");
        }
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
        //UNLOCK *********************************************** 
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr);
        close(logFileFd);
        _exit(EXIT_FAILURE);

    }


    yToZFifoFd = open(yToZFifoPath, O_WRONLY);// open fifo of the server y to server z
    if (yToZFifoFd == -1){
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        if (write(logFileFd,"serverFifoPath:\n",strlen("serverFifoPath:\n")) == -1){
            perror("failed to write into log file");
        }
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
        //UNLOCK *********************************************** 
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr);
        close(serverFd);
        close(logFileFd);
        _exit(EXIT_FAILURE);

    }
    // write(yToZFifoFd, &req, sizeof(struct request));

     
    dummyFd = open(serverFifoPath,O_WRONLY);// dummy fifo
    if (dummyFd == -1){
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        if (write(logFileFd,"dummyFd:\n",strlen("dummyFd:\n")) == -1){          
            perror("failed to write into log file");
        }
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
        //UNLOCK *********************************************** 

        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr);
        close(logFileFd);
        close(serverFd);
        _exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < poolSize1; ++i){// create pipes for each worker
        if (pipe(workerPipeArr[i]) == -1){
            // LOCK*************************************************
            fcntl(logFileFd,F_SETLKW,&lock);
            // LOCK*************************************************
            if (write(logFileFd,"pipe error:\n",strlen("pipe error:\n")) == -1){
                //UNLOCK ***********************************************
                lock.l_type = F_UNLCK;    
                fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
                //UNLOCK *********************************************** 
                perror("failed to write into log file");
                _exit(EXIT_FAILURE);
            }
            //UNLOCK ***********************************************
            lock.l_type = F_UNLCK;    
            fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
            //UNLOCK ***********************************************   
        }
    }

    if (sigintcaught == 1){// sig int

        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
        sprintf(printBufferPointer,"[%.19s] SIGINT recieved, terminating Z and exiting server Y. Total requests handeled: %d, %d invertable, %d not. %d requests were forwared.\n",ctime(&t),handeledInvReqs + handeledNotInvReqs,handeledInvReqs,handeledNotInvReqs,forwardedRequest);
        if (write(logFileFd,printBufferPointer,strlen(printBufferPointer)) == -1){
            //UNLOCK ***********************************************
            lock.l_type = F_UNLCK;    
            fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
            //UNLOCK ***********************************************

            perror("failed to write into log file");
            free(printBufferPointer);
        }
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);
        //UNLOCK ***********************************************
        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
        free(arr);    
        free(printBufferPointer);
        return 1;
    }

    // for (int i = 0; i < poolSize1; ++i){
        forkVal  = fork();
        switch(forkVal){
            case -1:
                perror("fork");
            case 0: // WORKERS PROCESSES CONTROL SEGMENT
                for (;;){
                    sleep(sleepDuration);
                    if (read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)){// read clients request into res struct.
                        continue;
                    }
                    printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
                    sprintf(printBufferPointer,"[%.19s] Worker PID#%d is handeling Client PID#%d, matrix size %dx%d, pool busy %d/%d\n",ctime(&t),getpid(),req.pid,req.matrixLen,req.matrixLen, 1,poolSize1);
                    // LOCK*************************************************
                    fcntl(logFileFd,F_SETLKW,&lock);
                    // LOCK*************************************************
                    write(logFileFd,printBufferPointer,strlen(printBufferPointer));
                    //UNLOCK ***********************************************
                    lock.l_type = F_UNLCK;    
                    fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
                    //UNLOCK *********************************************** 
                    free(printBufferPointer);
                    

                                
                    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) req.pid);
                    clientFd = open(clientFifo, O_WRONLY);// open response fifo
                    if (clientFd == -1){
                        // LOCK*************************************************
                        fcntl(logFileFd,F_SETLKW,&lock);
                        // LOCK*************************************************
                        if (write(logFileFd,"Can't Open clientFifo\n",strlen("Can't Open clientFifo\n")) == -1){
                            perror("failed to write into log file");
                        }
                        //UNLOCK ***********************************************
                        lock.l_type = F_UNLCK;    
                        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
                        //UNLOCK ***********************************************   
                        continue;
                    }

                    char *token;
                    token = strtok(req.payLoad,",");
                    int matrix[req.matrixLen][req.matrixLen];
                    for (int i = 0; i < req.matrixLen; ++i){// parse requsted matrix into 2D array
                        int j = 0;
                        while(j < req.matrixLen) {
                            matrix[i][j] = atoi(token);
                            token = strtok(NULL, ",");
                            j++;
                        }
                    }

                    if (determinantOfMatrix((int *)matrix, req.matrixLen) != 0){// if invertable
                        resp.isInvertable = 1;
                        handeledInvReqs++;
                    }
                    else{// if not invertable
                        resp.isInvertable = 0;
                        handeledNotInvReqs++;
                    }

                    // LOCK*************************************************
                    fcntl(logFileFd,F_SETLKW,&lock);
                    // LOCK*************************************************
                    if (write(clientFd, &resp,sizeof(struct response)) != sizeof(struct response)){// write into response fifo.
                        if (write(logFileFd,"Can't write into the fifo\n",strlen("Can't write into the fifo\n")) == -1){
                            perror("failed to write into log file");
                        }
                        //UNLOCK ***********************************************
                        lock.l_type = F_UNLCK;    
                        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
                        //UNLOCK ***********************************************  
                        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
                        free(arr);   
                         _exit(EXIT_FAILURE);
                    }
                    //UNLOCK ***********************************************
                    lock.l_type = F_UNLCK;    
                    fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
                    //UNLOCK ***********************************************  
                    if (close(clientFd) != -1){}


                    if (sigintcaught == 1){// sig int
                        printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
                        sprintf(printBufferPointer,"[%.19s] SIGINT recieved(WORKER), terminating Z and exiting server Y. Total requests handeled: %d, %d invertable, %d not. %d requests were forwared.\n",ctime(&t),handeledInvReqs + handeledNotInvReqs,handeledInvReqs,handeledNotInvReqs,forwardedRequest);
                        // LOCK*************************************************
                        fcntl(logFileFd,F_SETLKW,&lock);
                        // LOCK*************************************************
                        if (write(logFileFd,printBufferPointer,strlen(printBufferPointer)) == -1){
                            perror("failed to write into log file");
                            free(printBufferPointer);
                        }
                        //UNLOCK ***********************************************
                        lock.l_type = F_UNLCK;    
                        fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
                        //UNLOCK *********************************************** 
                        for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
                        free(arr);    
                        free(printBufferPointer);
                        close(logFileFd);         
                        close(serverFd);
                        close(dummyFd); 
                        return 1;
                    }
                }
            default:
                workerPID = forkVal;
                break;
        }
    // }
        

    for (;;){// SERVER Y CONTROL SEGMENT
        if (sigintcaught == 1){// sig int
            printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
            sprintf(printBufferPointer,"[%.19s] SIGINT recieved(SERVERY), terminating Z and exiting server Y. Total requests handeled: %d, %d invertable, %d not. %d requests were forwared.\n",ctime(&t),handeledInvReqs + handeledNotInvReqs,handeledInvReqs,handeledNotInvReqs,forwardedRequest);
            // LOCK*************************************************
            fcntl(logFileFd,F_SETLKW,&lock);
            // LOCK*************************************************
            if (write(logFileFd,printBufferPointer,strlen(printBufferPointer)) == -1){
                perror("failed to write into log file");
                free(printBufferPointer);
            }
            //UNLOCK ***********************************************
            lock.l_type = F_UNLCK;    
            fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
            //UNLOCK *********************************************** 
            for (int i = 0; i < serverZArgArrSize; i++){ free(arr[i]);}
            free(arr);    
            free(printBufferPointer);
            close(logFileFd);         
            close(serverFd);
            close(dummyFd);
            kill(SIGINT,workerPID); 
            return 1;
        } 
    }
	return 0;
}