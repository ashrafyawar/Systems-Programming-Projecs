#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>
#include <errno.h>
#include "helperfunctions.h"
#include <sys/wait.h>
#include<stdlib.h>
#include <signal.h>
#include <math.h>

#define BUFFER_LIMIT 30// buffer size used to read from file.
#define ACCESS_PERMISSION_FLAG_INPUT  O_RDONLY
#define ACCESS_PERMISSION_FLAG_OUTPUT O_RDWR
#define TOTAL_MATRIX_SIZE 9 // size of a cov matrix 3x3 = 9
#define MIN 10000000.0 // max number used to cal

#define MODE S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH
sig_atomic_t sigintcaught = 0;

void siginthandler(){
    int esaved = errno;
    sigintcaught = 1;
    errno = esaved;
}

int main(int argc, char **argv){
	
  	const char *input_path = NULL;
    char *output_path = NULL;
    int inpfd = 0,outfd = 0, numDead = 0,bytesread = 0,index = 0,counter = 0, num_one = 0,num_two = 0;
    unsigned char buf[BUFFER_LIMIT];
    unsigned int coordinates_int[BUFFER_LIMIT];
    float min = MIN;
    char* coordinates_char = NULL;
    pid_t childPid;
    mode_t mode = MODE;
    char** arr = (char**)malloc(1 * sizeof(char*)); // malloc used to store output file descriptor to send it to child.

    struct sigaction newact;
    newact.sa_handler = &siginthandler; /* set the new handler */
    newact.sa_flags = 0;
    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT,&newact, NULL) == -1)){
        perror("Failed to install SIGINT signal handler");
    }


    for (int i = 0; i < 1; i++){
        arr[i] = (char*)malloc(BUFFER_LIMIT * sizeof(char));
    }

    if (sigintcaught == 1){
        for (int i = 0; i < 1; i++){ free(arr[i]);}
        free(arr);
        _exit(EXIT_FAILURE);
    }
 
    // check if the user has entered sufficient arguments.
    if (argc < 5){
        perror("No Sufficient Parameters To Execute The Edit !!!\n");
        fprintf(stdout,"Usage: vg ./hw02compiled -i inputFilePath -o outputFilePath\n");
        return 1;
    }else if (argc > 5){
        perror("Too Much Parameters To Execute The Edit !!!\n");
        fprintf(stdout,"Usage: vg ./hw02compiled -i inputFilePath -o outputFilePath\n");
        return 1;

    }

    //file paths read from terminal
    input_path = argv[2];
    output_path = argv[4];

    //open the input file
    inpfd = open(input_path,ACCESS_PERMISSION_FLAG_INPUT,mode);
    if (inpfd == -1){// if file not found then print error on sdterr
        open_syscall_error_print();
        perror("Coudln't Open The Input File !!!\n");
        return 1;
    }

    outfd = open(output_path,ACCESS_PERMISSION_FLAG_OUTPUT,mode);
    if (outfd == -1){// if file not found then print error on sdterr
        open_syscall_error_print();
        perror("Coudln't Open The Output File !!!\n");
        return 1;
    }

    sprintf(arr[0],"%d",outfd);
    
    fprintf(stdout,"Process P rearding %s\n",input_path);
    index = 0;

    if (sigintcaught == 1){
        for (int i = 0; i < 1; i++){ free(arr[i]);}
        free(arr);
        if (close(inpfd) == -1){
            close_syscall_error_print();
            return 1;
        }
        if (close(outfd) == -1){
            close_syscall_error_print();
            return 1;
        }
        _exit(EXIT_FAILURE);
    }

    int* pid_arr = (int*) malloc(sizeof(int)* 1000000);

    int pid_count_ind = 0;
    for (;;){
        coordinates_char = (char*) malloc((BUFFER_LIMIT * BUFFER_LIMIT) * sizeof(char));

        while((bytesread = read(inpfd,buf,BUFFER_LIMIT)) == -1 && errno == EINTR);
        if (bytesread <= 0 || bytesread < BUFFER_LIMIT){// if end of the file (or there are less than 10 coordinates left).
            read_syscall_error_print();
            break;
        }

        for (int i = 0; i < BUFFER_LIMIT; ++i){
            coordinates_int[i] = (unsigned int)buf[i];
        }

        for (int i = 0; i < BUFFER_LIMIT; ++i){
            char* temp = (char*) malloc(BUFFER_LIMIT * sizeof(char));
            sprintf(temp,"%d ",coordinates_int[i]);
            strcat(coordinates_char,temp);
            free(temp);
        }

        if (sigintcaught == 1){
            for (int i = 0; i < 1; i++){ free(arr[i]);}
            free(arr);
            free(pid_arr);
            free(coordinates_char);
            if (close(inpfd) == -1){
            close_syscall_error_print();
            return 1;
            }
            if (close(outfd) == -1){
                close_syscall_error_print();
                return 1;
            }
            _exit(EXIT_FAILURE);
        }

        fprintf(stdout,"Created R_%d With",index + 1);
        counter = 0;
        while (counter < BUFFER_LIMIT){
            fprintf(stdout," (%d,%d,%d) ",coordinates_int[counter],coordinates_int[counter+1],coordinates_int[counter + 2]);
            if (counter+3 < BUFFER_LIMIT){
                fprintf(stdout,",");
            }
            counter = counter + 3;
        }
        
        fprintf(stdout,"\n");
        if (setenv("SAGOPA",coordinates_char,1) == -1){
            perror("setenv error");
            _exit(EXIT_FAILURE);
        }
        int forkval = fork();
        pid_arr[pid_count_ind] = forkval;        
        
        switch(forkval){

            case -1:
                free(pid_arr);
                perror("Forking Failed");
            case 0: //  child process area
                execv("./childcompiled",arr);
                _exit(EXIT_SUCCESS);
            default:
                break;
        }

        free(coordinates_char);
        coordinates_char = NULL;
        index++;
        pid_count_ind++;
    }

    if (sigintcaught == 1){
        for (int i = 0; i < pid_count_ind; ++i){
            kill(pid_arr[i],SIGINT);
        }

        free(pid_arr);
        if (close(inpfd) == -1){
            close_syscall_error_print();
            return 1;
        }
        if (close(outfd) == -1){
            close_syscall_error_print();
            return 1;
        }
        _exit(EXIT_FAILURE);

    }

    if (coordinates_char != NULL){ free(coordinates_char); }
    for (int i = 0; i < 1; i++){ free(arr[i]);}
    free(arr);

    
    for (;;){
        
        childPid = wait(NULL);
        
        if (childPid == -1){
            
            if (errno == ECHILD){// all child process has returned.  PARENT CODE AREA >>>>>>>>>>>>>>>>
                
                int output_file_len = 0;
                if (close(outfd) == -1){ // close the file
                    close_syscall_error_print();
                    return 1;
                }

                outfd = open(output_path,O_RDONLY,mode); // open the file
                if (outfd == -1){// if file not found then print error on sdterr
                    open_syscall_error_print();
                    perror("Coudln't Open The Output File !!!\n");
                    return 1;
                }

                fprintf(stdout,"Reached EOF, collecting outputs from %s\n",output_path);                    
                for (;;){

                    char* temp = (char*) malloc(BUFFER_LIMIT * sizeof(char));
                    while((bytesread = read(outfd,temp,BUFFER_LIMIT)) == -1 && errno == EINTR);
                    if (bytesread <= 0){
                        free(temp);
                        break;
                    }
                    output_file_len += strlen(temp);
                    free(temp);
                }

                // char parent_arr[output_file_len];
                char* parent_arr = (char*) malloc(output_file_len * sizeof(char)+1);
                char* parent_arr2 = (char*) malloc(output_file_len * sizeof(char)+1);
                if(lseek(outfd,0,SEEK_SET)== -1){
                    perror("lseek failed");
                    lseek_syscall_error_print();
                }
                
                for (;;){

                    char* temp = (char*) malloc(output_file_len * sizeof(char));
                    while((bytesread = read(outfd,temp,output_file_len)) == -1 && errno == EINTR);
                    if (bytesread <= 0){  free(temp);break;}
                    strcat(parent_arr,temp);
                    free(temp);
                }

                strcpy(parent_arr2,parent_arr);

                char* tokenn = NULL;
                tokenn = strtok(parent_arr, "\n ");
                int lent_count = 0;
                while( tokenn != NULL ) {
                    tokenn = strtok(NULL, "\n ");
                    lent_count++;
                }
                free(parent_arr);
                char *token = NULL;
                float *arr = (float*) malloc(sizeof(float) * lent_count);
                token = strtok(parent_arr2, "\n ");
                int ind = 0;
                while( token != NULL ) {
                    arr[ind] = atof(token);
                    token = strtok(NULL, "\n ");
                    ind++;
                }
                free(parent_arr2);
                
                float norm_arr[lent_count/TOTAL_MATRIX_SIZE],matrices[lent_count / TOTAL_MATRIX_SIZE][TOTAL_MATRIX_SIZE];
                counter = 0;
                int i = 0;
                while (counter < lent_count){
                    float one_matrix[TOTAL_MATRIX_SIZE];
                    for (int i = 0; i < TOTAL_MATRIX_SIZE; ++i){
                        one_matrix[i] = arr[counter];
                        counter++;
                    }
                    for (int j = 0; j < TOTAL_MATRIX_SIZE; ++j){
                        matrices[i][j] = one_matrix[j];
                    }
                    i++;

                }
                free(arr);
                
                for (int i = 0; i < lent_count / TOTAL_MATRIX_SIZE; ++i){
                    float one_3d_matrix[3][3];
                    one_3d_matrix[0][0] = matrices[i][0];
                    one_3d_matrix[0][1] = matrices[i][1];
                    one_3d_matrix[0][2] = matrices[i][2];

                    one_3d_matrix[1][0] = matrices[i][3];
                    one_3d_matrix[1][1] = matrices[i][4];
                    one_3d_matrix[1][2] = matrices[i][5];

                    one_3d_matrix[2][0] = matrices[i][6];
                    one_3d_matrix[2][1] = matrices[i][7];
                    one_3d_matrix[2][2] = matrices[i][8];

                    norm_arr[i] = frobeniusNorm(one_3d_matrix);
                }

                for (int i = 0; i < lent_count / TOTAL_MATRIX_SIZE; ++i){
                    for (int j = 0; j < lent_count / TOTAL_MATRIX_SIZE; ++j){
                    
                        if (fabs(norm_arr[i] - norm_arr[j]) <= min){
                            if (i != j){
                                min = fabs(norm_arr[i] - norm_arr[j]);
                                num_one  = i;
                                num_two = j;
                            }

                        }
                    }
                }

                fprintf(stdout,"The closest 2 matrices are ");
                for (int i = 0; i < 9; ++i){
                    fprintf(stdout,"%f ",matrices[num_one][i]);
                }
                fprintf(stdout,"\n");
                fprintf(stdout,"and ");
                                
                for (int i = 0; i < 9; ++i){
                    fprintf(stdout,"%f ",matrices[num_two][i]);
                }
                fprintf(stdout," and their distance is: %f\n",min);

                fprintf(stdout," -- DONE, PROGRAM EXECUTED SUCCESSFULLY :)\n");
                if (close(inpfd) == -1){
                    close_syscall_error_print();
                    return 1;
                }
                if (close(outfd) == -1){
                    close_syscall_error_print();
                    return 1;
                }
                if (coordinates_char != NULL){
                    free(coordinates_char); 
                }
                free(pid_arr);
                _exit(EXIT_SUCCESS);
            }else{
                perror("Wait Failed");
            }
        }
        numDead++;
    }
    
    return 0;
}