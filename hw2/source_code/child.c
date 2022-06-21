#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>
#include <errno.h>
#include "helperfunctions.h"
#include<stdlib.h>
#include <signal.h>
#include<math.h>

#define BUFFER_LIMIT 30
#define LIMIT 10
#define COL_LEN 3
#define LOCK_MODES F_WRLCK

sig_atomic_t sigintcaught = 0; // used as hanlder indicator

void siginthandler(){
    int esaved = errno;
    sigintcaught = 1;
    errno = esaved;
}

int main(int argc, char const *argv[]){

	
	int coords_int[BUFFER_LIMIT],array_x[LIMIT],array_y[LIMIT],array_z[LIMIT],byteswrite = 0,outputfd = 0,ind = 0,col_ind = 0,index = 0;
	double total_x = 0.0,total_y = 0.0,total_z = 0.0,total_xy = 0.0,total_xz = 0.0,total_yz = 0.0,cov_mat[COL_LEN][COL_LEN],temparr[COL_LEN * COL_LEN],avg_x = 0.0, avg_y = 0.0, avg_z = 0.0,var_x = 0.0,var_y = 0.0, var_z = 0.0,var_xy = 0.0,var_xz = 0.0, var_yz = 0.0;
	char* coords = getenv("SAGOPA"), *token,*temparr_char = NULL;

	outputfd = atoi(argv[0]);
	struct flock lock;

	struct sigaction newact;
    newact.sa_handler = &siginthandler; /* set the new handler */
    newact.sa_flags = 0;
    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT,&newact, NULL) == -1)){
        perror("Failed to install SIGINT signal handler");
	}
    
    if (sigintcaught == 1){
    	_exit(EXIT_FAILURE);
    }

	if (lseek(outputfd,0,SEEK_END) == -1){
		perror("lseek failed");

	}
   	
   	token = strtok(coords, " ");
   	
   	while( token != NULL ) {
    	coords_int[ind] = atoi(token);
    	token = strtok(NULL, " ");
    	ind++;
   	}

   	ind = 0;
   	col_ind = 0;
   	while(ind < BUFFER_LIMIT){
    	array_x[col_ind] = coords_int[ind];
    	array_y[col_ind] = coords_int[ind+1];
    	array_z[col_ind] = coords_int[ind+2];
   		ind = ind + 3;
   		col_ind += 1;
   	}

   	for (int i = 0; i < LIMIT; ++i){
   		total_x += array_x[i];
   		total_y += array_y[i];
   		total_z += array_z[i];
   	}

   	if (sigintcaught == 1){
    	_exit(EXIT_FAILURE);
    }

   	avg_x =(double) total_x / LIMIT;
   	avg_y = (double) total_y / LIMIT;
   	avg_z = (double) total_z / LIMIT;

   	total_x = 0.0;
   	total_y = 0.0;
   	total_z = 0.0;
   	
   	for (int i = 0; i < LIMIT; ++i){
   		total_x += pow((array_x[i] - avg_x),2);
   		total_y += pow((array_y[i] - avg_y),2);
   		total_z += pow((array_z[i] - avg_z),2);
   		total_xy += ((array_x[i] - avg_x) * (array_y[i] - avg_y));
   		total_xy += ((array_x[i] - avg_x) * (array_z[i] - avg_z));
   		total_yz += ((array_y[i] - avg_y) * (array_z[i] - avg_z));
   	}
    
    var_x = (double) total_x / LIMIT;
    var_y = (double) total_y / LIMIT;
    var_z = (double) total_z / LIMIT;
    var_xy = (double) total_xy / LIMIT;
    var_xz = (double) total_xz / LIMIT;
    var_yz = (double) total_yz / LIMIT;

    cov_mat[0][0] = var_x;
    cov_mat[0][1] = var_xy;
    cov_mat[0][2] = var_xz;
    cov_mat[1][0] = var_xy;
    cov_mat[1][1] = var_y;
    cov_mat[1][2] = var_yz;
    cov_mat[2][0] = var_xz;
    cov_mat[2][1] = var_yz;
    cov_mat[2][2] = var_z;
    
    if (sigintcaught == 1){
    	_exit(EXIT_FAILURE);
    }

    index = 0;
    for (int i = 0; i < COL_LEN; ++i){
    	for (int j = 0; j < COL_LEN; ++j){
    		temparr[index] = cov_mat[i][j];
    		index++;
    	}
	}

    if (sigintcaught == 1){
    	_exit(EXIT_FAILURE);
    }

	temparr_char = (char*) malloc((COL_LEN * COL_LEN * BUFFER_LIMIT) * sizeof(char));
    for (int i = 0; i < COL_LEN * COL_LEN; ++i){
        char* temp = (char*) malloc(BUFFER_LIMIT * sizeof(char));
        sprintf(temp,"%f ",temparr[i]);
        strcat(temparr_char,temp);
        free(temp);
    }
    if (sigintcaught == 1){
    	free(temparr_char);
    	_exit(EXIT_FAILURE);
    }
    //LOCK THE FILE
    memset(&lock,0,sizeof(lock));
    lock.l_type = LOCK_MODES;
    if (fcntl(outputfd,F_SETLKW,&lock) == -1){// lock the file
        return 1;
    }
	
	while((byteswrite = write(outputfd,temparr_char,strlen(temparr_char))) == -1 && errno == EINTR);
	free(temparr_char);
	temparr_char = NULL;
	
	if (byteswrite <= 0){
		perror("read failed");
		return 1;

	}
	
	if (sigintcaught == 1){
    	_exit(EXIT_FAILURE);
    }

	while((byteswrite = write(outputfd,"\n",1)) == -1 && errno == EINTR);
	if (byteswrite <= 0){
		perror("read failed");
		return 1;

	}

    // UNLOCK THE FILE
    lock.l_type = F_UNLCK;    
    if (fcntl(outputfd,F_SETLKW,&lock) == -1){// unlock the file
        if (close(outputfd) == -1){
            return 1;
        }
    }	

	return 0;
}