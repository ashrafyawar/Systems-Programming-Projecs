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

#define LOCK_MODES F_WRLCK
#define CLIENT_FIFO_TEMPLATE "response_%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)
#define BUFFER_LIMIT 300000

volatile sig_atomic_t sigintcaught = 0; // used as hanlder indicator

struct request{
    pid_t pid;
    int matrixLen;
    char payLoad[BUFFER_LIMIT];
};
struct response{
    int isInvertable;
};

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

    int printBufferPointerLen = 1000;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    const char* logFilePath = NULL;
    char yToZFifoPath[12];
    char* printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
    int poolSize2 = 0,yToZFifoFd = 0, dummyFd = 0, clientFd = 0,logFileFd = 0,sleepDuration = 0;
    int handeledInvReqs = 0, handeledNotInvReqs = 0,forwardedRequest = 0,forkVal = 0;
    struct request req;
    struct response resp;
    struct sigaction newact;
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    time_t t;   // not a primitive datatype
    time(&t);

    newact.sa_handler = &siginthandler; /* set the new handler */
    newact.sa_flags = 0;
    sigaction(SIGINT, &newact, NULL);

    //file paths read from terminal
    strcpy(yToZFifoPath,argv[0]);
    logFilePath = argv[1];
    poolSize2 = atoi(argv[2]);
    sleepDuration = atoi(argv[3]);
    
    logFileFd = open(logFilePath,O_WRONLY,O_CREAT | O_TRUNC,0644);
    if (logFileFd == -1){
        perror("logFileFd:");
    }

    sprintf(printBufferPointer,"[%.19s] Server Z (%s, poolSize = %d, sleepDuration = %d) Started\n",ctime(&t),logFilePath,poolSize2,sleepDuration);
    // LOCK*************************************************
    fcntl(logFileFd,F_SETLKW,&lock);
    // LOCK*************************************************
    if (write(logFileFd,printBufferPointer,strlen(printBufferPointer)) == -1){
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);
        //UNLOCK ***********************************************
        perror("failed to write into log file");
        free(printBufferPointer);
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
        fcntl(logFileFd,F_SETLKW,&lock);
        //UNLOCK ***********************************************

        close(logFileFd);
        _exit(EXIT_FAILURE);
    }
    
    // pid_t workersPidsArr[poolSize1]; // workers pids array
    int workerPipeArr[poolSize2][2]; // workers pipes array      
    
    yToZFifoFd = open(yToZFifoPath, O_RDONLY);// open fifo of the server
    if (yToZFifoFd == -1){
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        if (write(logFileFd,"yToZFifoPath:\n",strlen("yToZFifoPath:\n")) == -1){
            perror("failed to write into log file");
        }
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);
        //UNLOCK ***********************************************
        close(logFileFd);         
        _exit(EXIT_FAILURE);

    }

    dummyFd = open(yToZFifoPath,O_WRONLY);// dummy fifo
    if (dummyFd == -1){
        // LOCK*************************************************
        fcntl(logFileFd,F_SETLKW,&lock);
        // LOCK*************************************************
        if (write(logFileFd,"dummyFd:\n",strlen("dummyFd:\n")) == -1){          
            perror("failed to write into log file");
        }
        //UNLOCK ***********************************************
        lock.l_type = F_UNLCK;    
        fcntl(logFileFd,F_SETLKW,&lock);
        //UNLOCK ***********************************************
        close(logFileFd);         
        close(yToZFifoFd);         
        _exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < poolSize2; ++i){// create pipes for each worker
        if (pipe(workerPipeArr[i]) == -1){
            // LOCK*************************************************
            fcntl(logFileFd,F_SETLKW,&lock);
            // LOCK*************************************************
            if (write(logFileFd,"pipe error:\n",strlen("pipe error:\n")) == -1){
                //UNLOCK ***********************************************
                lock.l_type = F_UNLCK;    
                fcntl(logFileFd,F_SETLKW,&lock);
                //UNLOCK ***********************************************
                perror("failed to write into log file");
                _exit(EXIT_FAILURE);
            }
            //UNLOCK ***********************************************
            lock.l_type = F_UNLCK;    
            fcntl(logFileFd,F_SETLKW,&lock);
            //UNLOCK ***********************************************  
        }
    }

    // for (int i = 0; i < poolSize1; ++i){
        forkVal  = fork();
        switch(forkVal){
            case -1:
                perror("fork");
            case 0: // WORKERS PROCESSES CONTROL SEGMENT
                for (;;){
                    sleep(sleepDuration);

                    printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
                    sprintf(printBufferPointer,"[%.19s] z:Worker PID#%d is handeling Client PID#%d, matrix size %dx%d, pool busy %d/%d\n",ctime(&t),getpid(),req.pid,req.matrixLen,req.matrixLen, 1,poolSize2);
                    // LOCK*************************************************
                    fcntl(logFileFd,F_SETLKW,&lock);
                    // LOCK*************************************************
                    write(logFileFd,printBufferPointer,strlen(printBufferPointer));
                    //UNLOCK ***********************************************
                    lock.l_type = F_UNLCK;    
                    fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
                    //UNLOCK *********************************************** 
                    free(printBufferPointer);
                
                    if (read(yToZFifoFd, &req, sizeof(struct request)) != sizeof(struct request)){// read clients request into res struct.
                        continue;
                    }
                                
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
                         _exit(EXIT_FAILURE);
                    }
                    //UNLOCK ***********************************************
                    lock.l_type = F_UNLCK;    
                    fcntl(logFileFd,F_SETLKW,&lock);// unlock the file
                    //UNLOCK ***********************************************  
                    if (close(clientFd) != -1){}
                    
                    if (sigintcaught == 1){// sig int
                        printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
                        sprintf(printBufferPointer,"[%.19s] SIGINT recieved(WORKER), exiting server Z. Total requests handeled: %d, %d invertable, %d not. %d requests were forwared.\n",ctime(&t),handeledInvReqs + handeledNotInvReqs,handeledInvReqs,handeledNotInvReqs,forwardedRequest);
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

                        free(printBufferPointer);
                        close(logFileFd);         
                        close(yToZFifoFd);
                        close(dummyFd); 
                        return 1;
                    }
                }
            default:
                break;
        }
    // }

    // for (;;){// serverZ run for ever untill signal arrives.
    //     if (sigintcaught == 1){// sig int
    //         printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));
    //         sprintf(printBufferPointer,"[%.19s] SIGINT recieved, exiting server Z. Total requests handeled: %d, %d invertable, %d not. %d requests were forwared.\n",ctime(&t),handeledInvReqs + handeledNotInvReqs,handeledInvReqs,handeledNotInvReqs,forwardedRequest);
    //         // LOCK*************************************************
    //         fcntl(logFileFd,F_SETLKW,&lock);
    //         // LOCK*************************************************
    //         if (write(logFileFd,printBufferPointer,strlen(printBufferPointer)) == -1){
    //             perror("failed to write into log file");
    //             free(printBufferPointer);
    //         }
    //         //UNLOCK ***********************************************
    //         lock.l_type = F_UNLCK;    
    //         fcntl(logFileFd,F_SETLKW,&lock);
    //         //UNLOCK ***********************************************

    //         free(printBufferPointer);
    //         close(logFileFd);         
    //         close(yToZFifoFd);
    //         close(dummyFd); 
    //         return 1;
    //     }
    // }
    return 0;
}