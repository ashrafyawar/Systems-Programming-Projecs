#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
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
#include "servantUtil.h"
#include <sys/socket.h> 
#include <netdb.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

sem_t *sp;
int cityCount = 0,processID = 0,servantListeningPort;
struct city *cityArr;
char allCities[CITY_LIMIT] = {'\0'};
sig_atomic_t sigintcaught = 0;
pthread_t threadGlob;

int loadDataParser(char *path,int counter);
void cityTraverser(char* path,int start, int end);
void countCity(char* path,int start, int end);
void siginthandler(int s);
int isProcess( const char* proc_id, int cmd );
int checkCitTransactionExistance(int fromYearDate,int fromMonthDate, int fromDayDate,int toYearDate,int toMonthDate,int toDayDate, int i, int j);
void* myThread(void* ptr){//main thread
    int ptrLocal = *((int *) ptr),socketFd,reqSocketFD;
    ptrLocal += ptrLocal;
    char* message;
    time_t t;time(&t);
    struct sockaddr_in server_addres; 
    int len_of_addr = sizeof(struct sockaddr_in);


// SERVANT SOCKET SETTINGS START **************************************************************************************************

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
    server_addres.sin_port = htons(servantListeningPort); 

    /* Binds the created socket to given ip and port */
    if ((bind(socketFd, (struct sockaddr*)&server_addres, sizeof(server_addres))) <  0) { 
        perror(" socket bind failed.\n"); 
        exit(EXIT_FAILURE);
    } 
  
    /* Server listens */
    if ((listen(socketFd, 4096)) < 0) { 
        perror(" server listen failed.\n"); 
        exit(EXIT_FAILURE); 
    }
// SERVANT SOCKET SETTINGS END **************************************************************************************************
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Servant %d: listening at port %d\n",ctime(&t),processID,servantListeningPort);
    printMessage(message); 
    while(1){
        
        if (sigintcaught == 1){
            fprintf(stderr,"Servant %d revieved SIGINT, Exiting...\n",processID);
            free(ptr);
            return(NULL);   
        }

        reqSocketFD = accept(socketFd,(struct sockaddr*)&server_addres,(socklen_t *)&len_of_addr);
        if (reqSocketFD < 0){
            perror("servant accept failed");
            exit(EXIT_FAILURE);
        }
        if (sigintcaught == 1){
            fprintf(stderr,"Servant %d revieved SIGINT, Exiting...\n",processID);
            free(ptr);
            return(NULL);   
        }
        struct clientReqServantInfo responseStruct;
        if(read(reqSocketFD,&responseStruct, sizeof(struct clientReqServantInfo)) == -1){
          perror("error occures reading from client to server!!");
          exit(EXIT_FAILURE);
        }       
        int totalTran = 0;
        for (int i = 0; i < cityCount; ++i){
            for (int j = 0; j < FILE_COUNT; ++j){
                int res = checkCitTransactionExistance(responseStruct.fromYearDate,responseStruct.fromMonthDate,responseStruct.fromDayDate, responseStruct.toYearDate,responseStruct.toMonthDate,responseStruct.toDayDate,i,j);
                if (res == 1 ){
                    for (int k = 0; k < TRANSACTION_COUNT; ++k){
                        if ( 
                        cityArr[i].fileArr[j].transactionArr[k].transactionType[0]  == responseStruct.transactionType[0] &&
                        cityArr[i].fileArr[j].transactionArr[k].transactionType[1]  == responseStruct.transactionType[1] &&
                        cityArr[i].fileArr[j].transactionArr[k].transactionType[2]  == responseStruct.transactionType[2]
                            ){
                            totalTran =  totalTran +  1;
                        }                            
                    }                    
                }

            }
        }
        if (sigintcaught == 1){
            fprintf(stderr,"Servant %d revieved SIGINT, Exiting...\n",processID);
            free(ptr);
            return(NULL);   
        }
        const int buf = totalTran;
        if(write(reqSocketFD,&buf, sizeof(int)) == -1){
          perror("error occures reading from client to server!!");
          exit(EXIT_FAILURE);
        }
    }

    free(ptr);
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    
    setbuf(stdout, NULL);
    char *directoryPath = NULL,*citiesAsStr = NULL,*message,*IP = NULL,*token = NULL,*semName = "mySemaphore",*portFilePath = "lib/uniquePort.txt";
    int bytesread = 0,PORT,s,startCityInd = 0, finishCityInd = 0,semVal = 1,portFileFD = 0,socketID = 0;
    mode_t mode = MODE;
    struct sigaction newact;
    time_t t;time(&t);
	pthread_t mainThread;
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
    if (argc < 9){
        perror("No Sufficient Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./servant -d directoryPath -c 10-19 -r IP -p PORT\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }else if (argc > 9){
        perror("Too Much Parameters !!!\n");
        message = (char*) malloc(MESSAGE_LEN * sizeof(char));
        sprintf(message,"[%.19s] Usage: vg ./servant -d directoryPath -c 10-19 -r IP -p PORT\n",ctime(&t));
        printMessage(message); 
        exit(EXIT_FAILURE);
    }
    //********************************************************************************************

    //file paths read from terminal
    directoryPath = argv[2];// derectory path
    citiesAsStr = argv[4];// cities as string
    IP = argv[6];
    PORT = atoi(argv[8]);

    token = strtok(citiesAsStr, "-");
    startCityInd = atoi(token);
    token = strtok(NULL, "-");
    finishCityInd = atoi(token);

    if (sigintcaught == 1){// if sigint has recieved.
        exit(EXIT_FAILURE);
    }

    //********************************************************************************************

    // LOAD DATASET
    countCity(directoryPath,startCityInd,finishCityInd);
    struct city *cityArrFirst = malloc( sizeof(struct city) * cityCount);
    cityArr = cityArrFirst;
    cityTraverser(directoryPath,startCityInd,finishCityInd);
    loadDataParser(directoryPath,0);

    if (sigintcaught == 1){// if sigint has recieved.
       free(cityArrFirst);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < cityCount; ++i){
        strcat(allCities,cityArr[i].cityName);
        strcat(allCities," ");
    }

    processID = (int) get_pid_from_proc_self();
    message = (char*) malloc(MESSAGE_LEN * sizeof(char));
    sprintf(message,"[%.19s] Servant %d: loaded dataset, cities %s-%s \n",ctime(&t),processID,cityArr[cityCount-1].cityName,cityArr[0].cityName);
    printMessage(message);
    //******************************************************************************************** 

    // semaphore settings: START
    sp = sem_open(semName,O_CREAT, 0644, semVal); // create / open semaphore 
    if (sp == SEM_FAILED){
        perror("sem open");
        free(cityArrFirst);
        sem_unlink(semName);
        exit(EXIT_FAILURE);
    }
    // semaphore settings: END
    //******************************************************************************************** 

    //GENERATE PORT FOR SERVANT
    sem_wait(sp);
        char buf[10] = {'\0'};
        portFileFD = open(portFilePath,O_RDWR,mode);
        
        if (portFileFD == -1){// if file not found then print error on sdterr
            perror("Coudln't Open The Input File !!!\n");
            sem_post(sp);
            free(cityArrFirst);
            sem_unlink(semName);
            sem_close(sp);
            exit(EXIT_FAILURE);
        }
        
        while((bytesread = read(portFileFD,buf,10)) == -1 && errno == EINTR);// read bytes from the input file
        servantListeningPort = atoi(buf);
        sprintf(buf,"%d",servantListeningPort + 1);
        lseek(portFileFD,0,SEEK_SET);
        while((bytesread = write(portFileFD,buf,strlen(buf))) == -1 && errno == EINTR);// read bytes from the input file   

    sem_post(sp);
    //******************************************************************************************** 

    // SEND SERVANT INFO TO SERVER
    struct clientReqServantInfo servantInfoStruct;
    servantInfoStruct.servantPort = servantListeningPort;
    strcpy(servantInfoStruct.servantAvailableCities, allCities);
    servantInfoStruct.flag = 2;
    servantInfoStruct.pid = processID;
    strcpy(servantInfoStruct.startCity,cityArr[cityCount-1].cityName);
    strcpy(servantInfoStruct.endCity ,cityArr[0].cityName);

    socketID = openSocket(socketID,IP,PORT);
    send(socketID,&servantInfoStruct,sizeof(struct clientReqServantInfo),0);
    close(socketID);
    //******************************************************************************************** 

    int *arg = malloc(sizeof(*arg));
    if ( arg == NULL ) {
        perror("Couldn't allocate memory for arg.\n");
        close(portFileFD);
        free(cityArrFirst);
        sem_unlink(semName);
        sem_close(sp);
        exit(EXIT_FAILURE);
    }
    *arg = 0;
    s = pthread_create(&mainThread,NULL, myThread,arg);
    if( s != 0){
        perror("pthread_create() error");
        close(portFileFD);
        sem_unlink(semName);
        sem_close(sp);
        free(cityArrFirst);
        exit(EXIT_FAILURE);
    }

    threadGlob = mainThread;
    
    if (sigintcaught == 1){// if sigint has recieved.
        pthread_cancel(mainThread);
        close(portFileFD);
        sem_unlink(semName);
        free(cityArrFirst);

        sem_close(sp);
        exit(EXIT_FAILURE);
    }
    //******************************************************************************************** 

    s = pthread_join(mainThread,NULL);
    if (s != 0) {
        perror("pthread_join() error");
        close(portFileFD);
        sem_unlink(semName);
        sem_close(sp);
        free(cityArrFirst);

        exit(EXIT_FAILURE);
    }
    
    if (sigintcaught == 1){// if sigint has recieved.  
        pthread_cancel(mainThread);
        close(portFileFD);
        sem_unlink(semName);
        sem_close(sp);
        free(cityArrFirst);

        exit(EXIT_FAILURE);
    }
    
    free(cityArrFirst);
    sem_unlink(semName);
    sem_close(sp);
    close(portFileFD);
    pthread_exit(0);
}

void siginthandler(int s){
    int esaved = errno;
    sigintcaught = 1;
    errno = esaved;
}

void countCity(char* path,int start, int end){

    struct dirent **namelist;
    int n;
    n = scandir(path, &namelist, NULL, alphasort);
    if (n < 0){  perror("scandir");
    }else {
        while (n--) {
            if (strcmp(namelist[n]->d_name, ".") != 0 && strcmp(namelist[n]->d_name, "..") != 0) {
                if (n <= end + 1 && n >= start + 1){ ++cityCount;}
                free(namelist[n]);
            }
            else{ free(namelist[n]);}
        }
        free(namelist);
    }
}

void cityTraverser(char* path,int start, int end){

    struct dirent **namelist;
    int n;

    n = scandir(path, &namelist, NULL, alphasort);
    if (n < 0){ 
        perror("scandir");
    }else {
     
        int counter = 0;
        while (n--) {
            if (strcmp(namelist[n]->d_name, ".") != 0 && strcmp(namelist[n]->d_name, "..") != 0) {
                if (n <= end + 1 && n >= start + 1){
                    strcpy(cityArr[counter].cityName,namelist[n]->d_name);
                    ++counter;
                }
                free(namelist[n]);
            }else{ free(namelist[n]);}
        }
        free(namelist);
    }
}

int loadDataParser(char *path,int counter) {
    
    DIR *dir;
    struct dirent *entry;
    dir = opendir(path);

    if (dir == NULL) {
        perror("Failed to open directory.\n");
        closedir(dir);
        return 0;
    }
    int ind = 0;
    while ((entry = readdir(dir)) != NULL) {
    
        if(entry->d_type == DT_DIR) {// if inside a city.
        
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 ) {
                int isExist = 0;
                for (int i = 0; i < cityCount; ++i){// look for the mathcing city in the city list.
                    if (strcmp(entry->d_name, cityArr[i].cityName) == 0){
                        isExist = 1;
                        counter = i;
                        break;
                    }
                }
                if (isExist){
                    
                    char *curPath = (char *)malloc(strlen(path) + strlen(entry->d_name) + 3);
                    sprintf(curPath, "%s/%s", path, entry->d_name);
                    loadDataParser(curPath,counter);
                    free(curPath);
                }
            }
        
        } else {// if inside file list of a city
        
        int inpfd,bytesread = 0,tranInd = 0;
        char buf[MESSAGE_LEN * TRANSACTION_COUNT] = {'\0'},*token,*tranToken,*fileToken,*fileTokenPt;
        strcpy(cityArr[counter].fileArr[ind].fileName,entry->d_name);
        char *pa = (char *)malloc(strlen(path) + strlen(entry->d_name) + 3);
        sprintf(pa, "%s/%s", path, entry->d_name);
        inpfd = open(pa,O_RDONLY,0666);
        free(pa);

        fileTokenPt = malloc(sizeof(char) * strlen(entry->d_name));
        strcpy(fileTokenPt,entry->d_name);
        fileToken = strtok(fileTokenPt, "-");
        if (fileToken != NULL){
            cityArr[counter].fileArr[ind].day = atoi(fileToken);
        }
        fileToken = strtok(NULL, "-");
        if (fileToken != NULL){
            cityArr[counter].fileArr[ind].month = atoi(fileToken);
        }
        fileToken = strtok(NULL, "-");
        if (fileToken != NULL){
            cityArr[counter].fileArr[ind].year = atoi(fileToken);
        }
        free(fileTokenPt);        
        while((bytesread = read(inpfd,buf,MESSAGE_LEN * TRANSACTION_COUNT)) == -1 && errno == EINTR);// read bytes from the input file   
        token = strtok(buf, "\n");
        while( token != NULL ) {
            strcpy(cityArr[counter].fileArr[ind].transactionArr[tranInd].transactionAsString,token);
           ++tranInd;
            token = strtok(NULL, "\n");
        }

        for (int i = 0; i < TRANSACTION_COUNT; ++i){
            
            tranToken = strtok(cityArr[counter].fileArr[ind].transactionArr[i].transactionAsString, " ");
            if (tranToken != NULL){
                cityArr[counter].fileArr[ind].transactionArr[i].id = atoi(tranToken);
            }
            tranToken = strtok(NULL, " ");
            if (tranToken != NULL){
                strcpy(cityArr[counter].fileArr[ind].transactionArr[i].transactionType,tranToken);
            }
            tranToken = strtok(NULL, " ");
            if (tranToken != NULL){
                strcpy(cityArr[counter].fileArr[ind].transactionArr[i].transactionLocation,tranToken);
            }

            tranToken = strtok(NULL, " ");
            if (tranToken != NULL){
                cityArr[counter].fileArr[ind].transactionArr[i].serfaceSquare = atoi(tranToken);
            }

            tranToken = strtok(NULL, " ");
            if (tranToken != NULL){
                cityArr[counter].fileArr[ind].transactionArr[i].price = atoi(tranToken);
            }
        }

        ++ind;
      }
    }
    closedir(dir);
    return 0;
}
int checkCitTransactionExistance(int fromYearDate,int fromMonthDate, int fromDayDate,int toYearDate,int toMonthDate,int toDayDate, int i, int j){
 
    if (cityArr[i].fileArr[j].year < fromYearDate || cityArr[i].fileArr[j].year > toYearDate){
        return 0;
    }
    if (cityArr[i].fileArr[j].year == fromYearDate){
        if (cityArr[i].fileArr[j].month < fromMonthDate){
            return 0;
        }else if (cityArr[i].fileArr[j].month == fromMonthDate){
            if (cityArr[i].fileArr[j].day < fromDayDate){
                return 0;
            }
        }
    }

    if (cityArr[i].fileArr[j].year == toYearDate){
        if (cityArr[i].fileArr[j].month > toMonthDate){
            return 0;
        }else if (cityArr[i].fileArr[j].month == toMonthDate){
            if (cityArr[i].fileArr[j].day > toDayDate){
                return 0;
            }
        }
    }
    return 1;
}