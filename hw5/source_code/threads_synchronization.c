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

#define MESSAGE_LEN 1024
#define BUFFER_LIMIT 1// buffer size used to read from file.
#define ACCESS_PERMISSION_FLAG_INPUT  O_RDONLY
#define MODE S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH

sig_atomic_t sigintcaught = 0;

pthread_t *threadsArray=NULL;
pthread_mutex_t mtx;
pthread_cond_t cond;
int arrived = 0,M = 0,N;
uint64_t **matrixA = NULL,**matrixB = NULL,**matrixC = NULL;
struct indexKeeper{
    int actualIndex;
    int startingIndex;
};
struct dftStruct{
    float realNum;
    float imgNum;
};
struct indexKeeper *indKeeperArr = NULL;
struct dftStruct **dft2dArr = NULL;

void freeThreadsPointersArr();
int power(int x, int y);
void printMessage(char *message);
void freeMatrixes();
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

void* myThread(void* ptr){
    int ptrLocal = *((int *) ptr),startingInd = 0,gapeNum = power(2,N)/M;
    char* message;
    time_t t;time(&t);
    int size = power(2,N);
    clock_t tt;tt = clock();
    double time_taken;
    if (sigintcaught == 1){// if sigint has recieved.
        free(ptr);
        pthread_exit(NULL);
    }
    
    // find the starting index of the matrix for multiplification
    for (int i = 0; i < M; ++i){
        if (indKeeperArr[i].actualIndex == ptrLocal){
            startingInd = indKeeperArr[i].startingIndex;
        }
    }
    if (sigintcaught == 1){// if sigint has recieved.
        free(ptr);
        pthread_exit(NULL);
    }

    // calcualte the matrix C for this partecular thread's share
    for (int i = startingInd; i < startingInd + gapeNum; i++) {
        for (int j = 0; j < size; j++) {
            matrixC[i][j] = 0;
            for (int k = 0; k < size; k++){
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
    if (sigintcaught == 1){// if sigint has recieved.
        free(ptr);
        pthread_exit(NULL);
    }

    tt = clock() - tt;
    time_taken = ((double)tt)/CLOCKS_PER_SEC;
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Thread %d has reached the rendezvous point in %f seconds.\n",ctime(&t),ptrLocal,time_taken);
    printMessage(message); 

    // barrier part START
    pthread_mutex_lock(&mtx);
    ++arrived;
    while(arrived < M){pthread_cond_wait(&cond,&mtx);}
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mtx);
    // barrier part END
    
    tt = clock();
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Thread %d is advancing to the second part\n",ctime(&t),ptrLocal);
    printMessage(message); 
    
    // struct dftStruct **temp  = (struct dftStruct**) calloc(gapeNum, sizeof(struct dftStruct*));
    // for (int i = 0; i < gapeNum; ++i){
    //     temp[i] = (struct dftStruct* ) calloc(size,sizeof(struct dftStruct));
    // }
    struct dftStruct **temp = NULL;
    //calcualte the 2D DFT
    int c = 0;
    for(int i=startingInd;i<startingInd+gapeNum;i++){
        
        for(int j=0;j<size;j++){
        
            float ak=0,bk=0;
        
            for(int ii=0;ii<size;ii++){
        
                for(int jj=0;jj<size;jj++){
        
                    float x = -2.0*M_PI*i*ii/(float)size;
                    float y =- 2.0*M_PI*j*jj/(float)size;
                    ak += matrixC[ii][jj]*cos(x+y);
                    bk += matrixC[ii][jj]*1.0*sin(x+y);
                }
            }
        
            dft2dArr[i][j].realNum = ak;
            dft2dArr[i][j].imgNum = bk;
            
            // temp[c][j].realNum = ak;
            // temp[c][j].imgNum = bk;
        }
        ++c;
    }
    if (sigintcaught == 1){// if sigint has recieved.
        free(ptr);
        pthread_exit(&temp);
    }

    tt = clock() - tt;
    time_taken = ((double)tt)/CLOCKS_PER_SEC;
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Thread %d has finished the second part in %f seconds.\n",ctime(&t),ptrLocal,time_taken);
    printMessage(message); 
    
    free(ptr);

    pthread_exit(&temp);
}

int main(int argc, char **argv){
    
    setbuf(stdout, NULL);
    char *input_path1 = NULL,*input_path2 = NULL,*outputFile = NULL,*message;
    int inpfd1 = 0,inpfd2 = 0,outputFd = 0,s,bytesread = 0,matrixLen = 0,indCounter = 0;
    mode_t mode = MODE;
    struct sigaction newact;
    time_t t;time(&t);
    pthread_mutexattr_t mtxAttr;
    clock_t tt;
    uint8_t matrixHalfSize;
    double time_taken;

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
    if (argc < 11){
        perror("No Sufficient Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./hw5 -i inputFile1.txt -j inputFile2.txt -o outputFile.csv -n 4 -m 4\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }else if (argc > 11){
        perror("Too Much Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./hw5 -i inputFile1.txt -j inputFile2.txt -o outputFile.csv -n 4 -m 4\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }

    //file paths read from terminal
    input_path1 = argv[2];// input file path
    input_path2 = argv[4];// input file path
    outputFile = argv[6];// output file path
    N = atoi(argv[8]);
    M = atoi(argv[10]);

    if (N < 2){// if n is less than 2
        perror("n is smaller than 2 !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./hw5 -i inputFile1.txt -j inputFile2.txt -o outputFile.csv -n 4 -m 4\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }
    if (power(2,N) < M){// if m < pow(2,n)
        M = power(2,N);
    }
    if (power(2,N) % M !=0){// if n and m are not compatible
        perror("not power(2,N) % M !\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./hw5 -i inputFile1.txt -j inputFile2.txt -o outputFile.csv -n 4 -m 4\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }

    //input files settings:
    inpfd1 = open(input_path1,ACCESS_PERMISSION_FLAG_INPUT,mode);
    if (inpfd1 == -1){// if file not found then print error on sdterr
        perror("Coudln't Open The Input File1 !!!\n");
        exit(EXIT_FAILURE);
    }
    inpfd2 = open(input_path2,ACCESS_PERMISSION_FLAG_INPUT,mode);
    if (inpfd2 == -1){// if file not found then print error on sdterr
        perror("Coudln't Open The Input File2 !!!\n");
        close(inpfd1);
        exit(EXIT_FAILURE);
    }
    outputFd = open(outputFile,O_WRONLY | O_CREAT,mode);
    if (outputFd == -1){// if file not found then print error on sdterr
        perror("Coudln't Open The Output !!!\n");
        close(inpfd1);close(inpfd2);
        exit(EXIT_FAILURE);
    }
    //********************************************************************************************
    
    // read from file1
    matrixLen = power(2,N) * power(2,N);
    char buf1[matrixLen];
    while((bytesread = read(inpfd1,buf1,matrixLen)) == -1 && errno == EINTR);// read bytes from the input file
    if (bytesread != matrixLen){// if the end of file is reached.
        perror("no sufficient data on file1 !!!");
        close(inpfd1);close(inpfd2);close(outputFd);
        exit(EXIT_FAILURE);
    }
    
    // read from file2
    char buf2[matrixLen];
    bytesread = 0;
    while((bytesread = read(inpfd2,buf2,matrixLen)) == -1 && errno == EINTR);// read bytes from the input file
    if (bytesread != matrixLen){// if the end of file is reached.
        perror("no sufficient data on file2 !!!");
        close(inpfd1);close(inpfd2);close(outputFd);
        exit(EXIT_FAILURE);
    }
    matrixHalfSize = power(2,N),indCounter = 0;
    
    // initialize and fill in MatrixA
    matrixA = (uint64_t**) calloc(matrixHalfSize, sizeof(uint64_t*));
    for (int i = 0; i < matrixHalfSize; ++i){
        matrixA[i] = (uint64_t* ) calloc(matrixHalfSize,sizeof(uint64_t));
    }

    for (int i = 0; i < matrixHalfSize; ++i){
        for (int j = 0; j < matrixHalfSize; ++j){
            matrixA[i][j] = buf1[indCounter];
            ++indCounter;
        }
    }
    
    // initialize and fill in MatrixB
    indCounter = 0;
    matrixB = (uint64_t**) calloc(matrixHalfSize, sizeof(uint64_t*));
    for (int i = 0; i < matrixHalfSize; ++i){
        matrixB[i] = (uint64_t* ) calloc(matrixHalfSize,sizeof(uint64_t));
    }
    for (int i = 0; i < matrixHalfSize; ++i){
        for (int j = 0; j < matrixHalfSize; ++j){
            matrixB[i][j] = buf2[indCounter];
            ++indCounter;
        }
    }
    // initialize MatrixC
    matrixC = (uint64_t**) calloc(matrixHalfSize, sizeof(uint64_t*));
    for (int i = 0; i < matrixHalfSize; ++i){
        matrixC[i] = (uint64_t* ) calloc(matrixHalfSize,sizeof(uint64_t));
    }

    // initialize dft2dArr
    dft2dArr = (struct dftStruct**) calloc(matrixHalfSize, sizeof(struct dftStruct*));
    for (int i = 0; i < matrixHalfSize; ++i){
        dft2dArr[i] = (struct dftStruct* ) calloc(matrixHalfSize,sizeof(struct dftStruct));
    }
    
    // initialize indKeeperArr
    indKeeperArr = (struct indexKeeper*) calloc(matrixHalfSize,sizeof(struct indexKeeper));
    //********************************************************************************************

    // mutex initialization settings:
    s = pthread_mutexattr_init(&mtxAttr);
    if (s != 0){
        perror("pthread_mutexattr_init");
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        exit(EXIT_FAILURE);
    }
    s = pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_ERRORCHECK);
    if (s != 0){
        perror("pthread_mutexattr_settype");
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        exit(EXIT_FAILURE);
    }
    s = pthread_mutex_init(&mtx, &mtxAttr);
    if (s != 0){
        perror("pthread_mutex_init");
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        exit(EXIT_FAILURE);
    }
    s = pthread_mutexattr_destroy(&mtxAttr);
    if (s != 0){
        perror("pthread_mutexattr_destroy");
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 
    
    // condition variable initialization settings:
    s = pthread_cond_init(&cond, NULL);
    if (s != 0){
        perror("pthread_cond_init");
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 

    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Two matrices of size %dx%d have read. The number of threads is %d\n",ctime(&t),power(2,N),power(2,N),M);
    printMessage(message);
    
    // fill in the indKeeperArr:
    int startingIndCounter = 0,gapeNum = (power(2,N)/M);
    for (int i = 0; i < M; ++i){
        indKeeperArr[i].actualIndex = i;
        indKeeperArr[i].startingIndex = startingIndCounter;
        startingIndCounter +=gapeNum;
    }

    // threads creation settings:
    tt = clock(); // start the timer
    pthread_t threadsPointersList[M];
    for (int i = 0; i < M; ++i){

        int *arg = malloc(sizeof(*arg));
        if ( arg == NULL ) {
            perror("Couldn't allocate memory for arg.\n");
            close(inpfd1);close(inpfd2);close(outputFd);
            freeMatrixes();
            free(indKeeperArr);
            exit(EXIT_FAILURE);
        }
        *arg = i;
        s = pthread_create(&threadsPointersList[i],NULL, myThread,arg);
        if( s != 0){
            perror("pthread_create() error");
            close(inpfd1);close(inpfd2);close(outputFd);
            freeMatrixes();
            free(indKeeperArr);
            exit(EXIT_FAILURE);
        }
    }
    threadsArray = threadsPointersList;
    
    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < M; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        freeThreadsPointersArr();
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 

    // wait for the threads to terminate:
    // void **temp = calloc(power(2,N)/M, sizeof(void*));
    // for (int i = 0; i < power(2,N)/M; ++i){
    //     temp[i] = (struct dftStruct* ) calloc(matrixHalfSize,sizeof(struct dftStruct));
    // }
    for (int i = 0; i < M; ++i){
        s = pthread_join(threadsPointersList[i],NULL);
        if (s != 0) {
            perror("pthread_join() error");
            close(inpfd1);close(inpfd2);close(outputFd);
            freeThreadsPointersArr();
            freeMatrixes();
            free(indKeeperArr);
            exit(EXIT_FAILURE);
        }
        
        // struct dftStruct **tempoooo = (struct dftStruct**) temp;
        
        // for (int i = 0; i < power(2,N)/M; ++i){
        //     for (int j = 0; j < matrixHalfSize; ++j){
        //         printf("%f ",tempoooo[i][j].realNum);
        //     }
        //     printf("\n");
        // }
        // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        // for (int i = 0; i < power(2,N); ++i){
        //     free(temp[i]);
        // }
        // free(temp);
    }
    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < M; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        freeThreadsPointersArr();
        exit(EXIT_FAILURE);
    }
    //********************************************************************************************

    //print output into the outputFile.csv
    for (int i = 0; i < matrixHalfSize; ++i){
        int len = 0;

        for (int j = 0; j < matrixHalfSize; ++j){

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            if (dft2dArr[i][j].imgNum >= 0){
                sprintf(message,"%0.3fi + %0.3fj,",dft2dArr[i][j].realNum,dft2dArr[i][j].imgNum);
                len = strlen(message);
                while(write(outputFd,message,len) == -1 && errno == EINTR);
            }else{
                sprintf(message,"%0.3fi %0.3fj,",dft2dArr[i][j].realNum,dft2dArr[i][j].imgNum);
                len = strlen(message);
                while(write(outputFd,message,len) == -1 && errno == EINTR);
            }
            free(message);
        }

        while(write(outputFd,"\n",1) == -1 && errno == EINTR);
    }
    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < M; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        freeThreadsPointersArr();
        exit(EXIT_FAILURE);
    }
    //********************************************************************************************
    
    tt = clock() - tt;// stope the timer
    time_taken = ((double)tt)/CLOCKS_PER_SEC;
    
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] The process has written the output file. The Total time spent is %f seconds.\n",ctime(&t),time_taken);
    printMessage(message);

    // for (int i = 0; i < matrixHalfSize; ++i){
    //     for (int j = 0; j < matrixHalfSize; ++j){
    //        printf("%-7ld ",matrixC[i][j]); 
    //     }
    //     printf("\n");
    // }
    
    s = pthread_mutex_destroy(&mtx);
    if (s != 0){
        perror("pthread_mutex_destroy");
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        freeThreadsPointersArr();
        exit(EXIT_FAILURE);
    }
    
    s = pthread_cond_destroy(&cond);
    if (s != 0){
        perror("pthread_cond_destroy");
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        freeThreadsPointersArr();
        free(indKeeperArr);
        exit(EXIT_FAILURE);
    }

    if (sigintcaught == 1){// if sigint has recieved.
        for (int i = 0; i < M; ++i){        
            pthread_cancel(threadsPointersList[i]);
        }
        close(inpfd1);close(inpfd2);close(outputFd);
        freeMatrixes();
        free(indKeeperArr);
        freeThreadsPointersArr();
        exit(EXIT_FAILURE);
    }
    
    close(inpfd1);close(inpfd2);close(outputFd);
    freeMatrixes();
    free(indKeeperArr);
    // freeThreadsPointersArr();
    pthread_exit(0);
}

void printMessage(char *message){
    while(((write(STDERR_FILENO, message, strlen(message))) == -1) && (errno == EINTR));
    free(message);
}

int power (int x, int y){  
    int power = 1, i; 
    for (i = 1; i <= y; ++i){  power = power * x;}  
    return power;  
}

void freeMatrixes(){
    int size = power(2,N);
    
    //free A
    for(int i = 0; i < size; i++){free(matrixA[i]);}
    free(matrixA);
    //free B
    for(int i = 0; i < size; i++){free(matrixB[i]);}
    free(matrixB);
    //free C
    for(int i = 0; i < size; i++){free(matrixC[i]);}
    free(matrixC);
    //free dft2dArr
    for (int i = 0; i < size; ++i){ free(dft2dArr[i]);}
    free(dft2dArr);
}
void freeThreadsPointersArr(){
    free(threadsArray);
}