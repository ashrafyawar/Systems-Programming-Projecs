#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include<stdlib.h>
#include <signal.h>
#include <semaphore.h>

#define BUFFER_LIMIT 3// buffer size used to read from file.
#define SHARED_MEM_LEN 2
#define SHARED_MEM_LEN_ING 1
#define MESSAGE_LEN 1000
#define ACCESS_PERMISSION_FLAG_INPUT  O_RDONLY
#define MODE S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH
#define SHARED_MEM_NAME "/shared_mem"
sig_atomic_t sigintcaught = 0;
sem_t *sp;
int childPidsArr[6];

void open_syscall_error_print();
void read_syscall_error_print();
void printMessage(char *message);
void siginthandler(){
    int esaved = errno;
    sigintcaught = 1;
    errno = esaved;
}

int chef0(int chefInd,char *addr){
    int totalDesNum = 0,printonce = 0;
    char tem1[SHARED_MEM_LEN],*message;
    for (;;){
        if (sigintcaught == 1){exit(EXIT_FAILURE);}
        if (printonce == 0){
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Walnuts and Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            printonce = 1;
        }
        sem_wait(sp);
        if (addr[0] == 'i'){ // if initially whole seller didn't put ingredient yet.
            sem_post(sp);
            continue;
        }
        else if (addr[0] =='n'){ // if whole seller has finished bringing ingredients

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is exiting\n",chefInd,getpid());
            printMessage(message);
            sem_post(sp);
            return totalDesNum;
        }else if (addr[0] == 'W' && addr[1] == 'S'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);

            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Walnuts and Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            
            sem_post(sp); 
        }else if (addr[0] == 'S' && addr[1] == 'W'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            totalDesNum++;
           
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Walnuts and Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp); 
        }
        else{
            sem_post(sp);
        }
    }

    return totalDesNum;
}
int chef1(int chefInd,char *addr){
    int totalDesNum = 0,printonce = 0;
    char tem1[SHARED_MEM_LEN],*message;
    for (;;){
        if (sigintcaught == 1){exit(EXIT_FAILURE);}
        if (printonce == 0){

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Flour and Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            printonce = 1;
        }
        sem_wait(sp);
        if (addr[0] == 'i'){ // if initially whole seller didn't put ingredient yet.
            sem_post(sp);
            continue;
        }
        else if (addr[0] =='n'){ // if whole seller has finished bringing ingredients

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is exiting\n",chefInd,getpid());
            printMessage(message); 
            sem_post(sp);
            return totalDesNum;
        }else if (addr[0] == 'F' && addr[1] == 'W'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Flour and Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp); 
        }else if (addr[0] == 'W' && addr[1] == 'F'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Flour and Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp); 
        }
        else{
            sem_post(sp);
        }
    }
    return totalDesNum;
}
int chef2(int chefInd,char *addr){
    int totalDesNum = 0,printonce = 0;
    char tem1[SHARED_MEM_LEN],*message;
    for (;;){
        if (sigintcaught == 1){exit(EXIT_FAILURE);}
        if (printonce == 0){
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Sugar and Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            printonce = 1;
        }
        sem_wait(sp);
        if (addr[0] == 'i'){ // if initially whole seller didn't put ingredient yet.
            sem_post(sp);
            continue;
        }
        else if (addr[0] =='n'){ // if whole seller has finished bringing ingredients
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is exiting\n",chefInd,getpid());
            printMessage(message); 
            sem_post(sp);
            return totalDesNum;
        }else if (addr[0] == 'S' && addr[1] == 'F'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Sugar and Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp); 
        }else if (addr[0] == 'F' && addr[1] == 'S'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Sugar and Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp);  
        }
        else{
            sem_post(sp);
        }
    }
    return totalDesNum;
}
int chef3(int chefInd,char *addr){
    int totalDesNum = 0,printonce = 0;
    char tem1[SHARED_MEM_LEN],*message;
    for (;;){
        if (sigintcaught == 1){exit(EXIT_FAILURE);}
        if (printonce == 0){
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Milk and Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            printonce = 1;
        }
        sem_wait(sp);
        if (addr[0] == 'i'){ // if initially whole seller didn't put ingredient yet.
            sem_post(sp);
            continue;
        }
        else if (addr[0] =='n'){ // if whole seller has finished bringing ingredients
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is exiting\n",chefInd,getpid());
            printMessage(message); 
            sem_post(sp);
            return totalDesNum;
        }else if (addr[0] == 'M' && addr[1] == 'F'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Milk and Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp);  
        }else if (addr[0] == 'F' && addr[1] == 'M'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Flour >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Flour and Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp); 
        }
        else{
            sem_post(sp);
        }
    }
    return totalDesNum;
}
int chef4(int chefInd,char *addr){
    int totalDesNum = 0,printonce = 0;
    char tem1[SHARED_MEM_LEN],*message;
    for (;;){
        if (sigintcaught == 1){exit(EXIT_FAILURE);}
        if (printonce == 0){
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Milk and Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            printonce = 1;
        }
        sem_wait(sp);
        if (addr[0] == 'i'){ // if initially whole seller didn't put ingredient yet.
            sem_post(sp);
            continue;
        }
        else if (addr[0] =='n'){ // if whole seller has finished bringing ingredients
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is exiting\n",chefInd,getpid());
            printMessage(message); 
            sem_post(sp);
            return totalDesNum;
        }else if (addr[0] == 'M' && addr[1] == 'W'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Flour and Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp);
        }else if (addr[0] == 'W' && addr[1] == 'M'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Milk and Walnuts >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp);
        }
        else{
            sem_post(sp);
        }
    }
    return totalDesNum;
}
int chef5(int chefInd,char *addr){
    int totalDesNum = 0,printonce = 0;
    char tem1[SHARED_MEM_LEN],*message;
    for (;;){
        if (sigintcaught == 1){exit(EXIT_FAILURE);}
        if (printonce == 0){
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Sugar and Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message);
            printonce = 1;
        }
        sem_wait(sp);
        if (addr[0] == 'i'){ // if initially whole seller didn't put ingredient yet.
            sem_post(sp);
            continue;
        }
        else if (addr[0] =='n'){ // if whole seller has finished bringing ingredients
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is exiting\n",chefInd,getpid());
            printMessage(message); 
            sem_post(sp);
            return totalDesNum;
        }else if (addr[0] == 'S' && addr[1] == 'M'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Sugar and Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp);
        }else if (addr[0] == 'M' && addr[1] == 'S'){
            tem1[0] = 't';
            tem1[1] = addr[1];
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Milk >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            tem1[1] = 't';
            memcpy(addr,tem1,SHARED_MEM_LEN);

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has taken Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is preparing the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) has delivered the dessert >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            
            totalDesNum++;
            
            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"chef%d (pid %d) is waiting for Milk and Sugar >>> current ingredients: [%c] [%c]\n",chefInd,getpid(),addr[0],addr[1]);
            printMessage(message); 
            sem_post(sp);
        }
        else{
            sem_post(sp);
        }
    }
    return totalDesNum;
}

int main(int argc, char **argv){
    
    const char *input_path = NULL,*semName = NULL;
    int inpfd = 0,bytesread = 0,chefCount = 6,forkVal = -1,allDeserts = 0,sharedMemFd,pid = getpid(),status = 0;
    unsigned char buf[BUFFER_LIMIT];
    mode_t mode = MODE;
    unsigned int semVal = 1;
    char bufferArray[2],*addr,*message;   
    bufferArray[0] = bufferArray[1] = 'i';
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
        sprintf(message,"Usage: vg ./hw3named -i inputFilePath -n name\n");
        printMessage(message);

        exit(EXIT_FAILURE);
    }else if (argc > 5){
        perror("Too Much Parameters !!!\n");

        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"Usage: vg ./hw3named -i inputFilePath -n name\n");
        printMessage(message);
        
        exit(EXIT_FAILURE);
    }

    //file paths read from terminal
    input_path = argv[2];// input file path
    semName = argv[4];//semaphore name
    
    //input file settings:
    inpfd = open(input_path,ACCESS_PERMISSION_FLAG_INPUT,mode);
    if (inpfd == -1){// if file not found then print error on sdterr
        open_syscall_error_print();
        perror("Coudln't Open The Input File !!!\n");
        exit(EXIT_FAILURE);
    }

    // shared memory settings: START
    sharedMemFd = shm_open(SHARED_MEM_NAME,O_RDWR | O_CREAT,0666);
    if (sharedMemFd == -1){
        perror("sharedMemFd");
        close(inpfd);
        exit(EXIT_FAILURE);
    }
    if (ftruncate(sharedMemFd,SHARED_MEM_LEN) == -1){
        perror("ftruncate");
        close(sharedMemFd);
        close(inpfd);
        exit(EXIT_FAILURE);
    }
    addr = mmap(NULL,SHARED_MEM_LEN,PROT_READ | PROT_WRITE,MAP_SHARED,sharedMemFd,0);
    if (addr == MAP_FAILED){
        perror("addr");
        close(sharedMemFd);
        close(inpfd);
        exit(EXIT_FAILURE);
    }
    if (close(sharedMemFd) == -1){
        perror("close");
        close(inpfd);
        exit(EXIT_FAILURE);
    }
    // shared memory settings: END
    memcpy(addr,bufferArray,SHARED_MEM_LEN);
    
    // semaphore settings: START
    sp = sem_open(semName,O_CREAT, 0644, semVal); // create / open semaphore 
    if (sp == SEM_FAILED){
        perror("sem open");
        close(sharedMemFd);
        close(inpfd);
        exit(EXIT_FAILURE);
    }
    // semaphore settings: END

    if (sigintcaught == 1){
        close(sharedMemFd);
        close(inpfd);
        sem_close(sp);
        sem_unlink(semName);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < chefCount; ++i){
        
        forkVal = fork();
        if (forkVal == -1){// if error occured.
            perror("fork:");
            close(sharedMemFd);
            close(inpfd);
            sem_close(sp);
            sem_unlink(semName);
            exit(EXIT_FAILURE);
        }else if (forkVal == 0){ // if child processes
            childPidsArr[i] = getpid(); // store childs pids.
            if (i == 0 ){ // if chef0
                return chef0(i,addr);
            }else if(i == 1){// if chef1
                return chef1(i,addr);
            }else if(i == 2){// if chef2
                return chef2(i,addr);
               
            } else if(i == 3){// if chef3
                return chef3(i,addr);
            } else if(i == 4){// if chef4
                return chef4(i,addr);   
            } else if(i == 5){// if chef5
                return chef5(i,addr);
            }                  
        }
    }

    for (;;){// whole seller process code segment
        
        sem_wait(sp);// take the semaphore
        
        if ((addr[0] != 'i' && addr[1] != 'i') && (addr[0] != 't' && addr[1] != 't')){// if the table is not empty of ingredients and it's not the wholer sellers first time bringing ings.
            sem_post(sp);// release the semaphore
            continue;
        }
        
        if (addr[0] == 't' && addr[1] == 't'){// if the table is empty

            message = (char*) malloc(MESSAGE_LEN * sizeof(char));
            sprintf(message,"the wholesaler (pid %d) has obtained the dessert and left >>> current ingredients: [%c] [%c]\n",pid,addr[0],addr[1]);
            printMessage(message);
        }
        
        while((bytesread = read(inpfd,buf,BUFFER_LIMIT)) == -1 && errno == EINTR);// read bytes from the input file
        
        if (bytesread <= 0 ){// if the end of file is reached.
            read_syscall_error_print();
            char * temp = "nn";
            memcpy(addr,temp,SHARED_MEM_LEN);
            sem_post(sp);
            break;
        }
        
        memcpy(addr,buf,SHARED_MEM_LEN);
        
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"the wholesaler (pid %d) delivers [%c] and [%c]\n",getpid(),addr[0],addr[1]);
        printMessage(message);
        
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"the wholesaler (pid %d) is waiting for the dessert >>> current ingredients: [%c] [%c]\n",pid,addr[0],addr[1]);
        printMessage(message);

        sem_post(sp);

        if (sigintcaught == 1){// if sigint recieved.
            close(sharedMemFd);
            close(inpfd);
            sem_close(sp);
            sem_unlink(semName);
            exit(EXIT_FAILURE);
        }
    }

    if (sigintcaught == 1){// if sigint recieved
        close(sharedMemFd);
        close(inpfd);
        sem_close(sp);
        sem_unlink(semName);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < chefCount; ++i){// collect the return values of the child processes
        int childPid = waitpid(childPidsArr[i],&status,0);
        if (childPid == -1){
            perror("error while waitpid()");
            close(sharedMemFd);
            close(inpfd);
            sem_close(sp);
            sem_unlink(semName);
            exit(EXIT_FAILURE);
        }
        allDeserts  = allDeserts + WEXITSTATUS(status);
    }
    
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"the wholesaler (pid %d) is done (total desserts: %d)\n",pid,allDeserts);
    printMessage(message);

    close(sharedMemFd);
    close(inpfd);
    sem_close(sp);
    sem_unlink(semName);
    return 0;
}
    
void open_syscall_error_print(){
    
    if (errno == EACCES){
        perror("The requested access to the file is not allowed!!!\n");
    }else if (errno == EDQUOT){
        perror("Where O_CREAT is specified, the file does not exist!!!\n");
    }else if (errno == EEXIST){
        perror("Pathname already exists and O_CREAT and O_EXCL were used!!!\n");
    }else if (errno == EFAULT){
        perror("Pathname points outside your accessible address space!!!\n");
    }else if (errno == EFBIG){
        perror("See EOVERFLOW!!!\n");
    }else if (errno == EINTR){
        perror("While  blocked  waiting  to  complete  an  open  of a slow device!!!\n");
    }else if (errno == EINVAL){
        perror("Invalid value in flags!!!\n");
    }else if (errno == ENOTDIR){
        perror("Pathname  is  a relative pathname and dirfd is a file descriptor referring to a file other than a directory!!!\n");
    }
}
void read_syscall_error_print(){
    
    if (errno == EAGAIN){
        perror("The file descriptor fd refers to a file other than a socket and has been marked non‐blocking!!!\n");
    }else if (errno == EBADF){
        perror("fd is not a valid file descriptor or is not open for reading!!!\n");
    }else if (errno == EFAULT){
        perror("buf is outside your accessible address space!!!\n");
    }else if (errno == EINTR){
        perror("The call was interrupted by a signal before any data was read!!!\n");
    }else if (errno == EINVAL){
        perror("fd is attached to an object which is unsuitable for reading; or the file was  opened with the O_DIRECT flag, and either the address specified in buf, the value specified in count, or the file offset is not suitably aligned!!!\n");
    }else if (errno == EIO){
        perror("I/O error!!!\n");
    }else if (errno == EISDIR){
        perror("fd refers to a directory!!!\n");
    }
}
void printMessage(char *message){
    while(((write(STDERR_FILENO, message, strlen(message))) == -1) && (errno == EINTR));
    free(message);
}