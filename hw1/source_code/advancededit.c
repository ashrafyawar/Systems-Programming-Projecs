#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>
#include <errno.h>
#include "helperfunctions.h"

// some macros for the program
#define ACCESS_PERMISSION_FLAG  O_RDWR
#define MODE S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH
#define LOCK_MODES F_WRLCK | F_RDLCK

int main(int argc, char **argv){
	
  	const char *path = NULL;
    int fd = 0,counter = 0,str_pair_counter = 1;
    char *token = NULL,*sub_token = NULL,*string = NULL,*str1 = NULL,*str2 = NULL, *case_insensetive = NULL;
    int case_insensetive_flag  = 1;
    // read, write and execution mode for users, groups and others.
    mode_t mode = MODE;
    struct flock lock;
    
    // check if the user has entered sufficient arguments.
    if (argc < 3){
        fprintf(stderr, "No Sufficient Parameters To Execute The Edit !!!\n");
        fprintf(stderr, "Usage: vg ./editcompiled '/string_to _replace_to/string_to_replace_with/' inputFilePath\n");
        return 1;
    }else if (argc > 3){
        fprintf(stderr, "Too Much Parameters To Execute The Edit !!!\n");
        fprintf(stderr, "Usage: vg ./editcompiled '/string_to _replace_to/string_to_replace_with/' inputFilePath\n");
        return 1;

    }

    // file path read from terminal
    path = argv[2];

    //open the file
    fd = open(path,ACCESS_PERMISSION_FLAG,mode);
    if (fd == -1){// if file not found then print error on sdterr
        open_syscall_error_print();
        fprintf(stderr, "Coudln't Open The File !!!\n");
        return 1;
    }

    string = argv[1];
    string[strlen(string)] = '\0';

    token  = strchr(string,';');

    fprintf(stdout," -- Waiting For The File LOCK To Be Available...\n");
    
    memset(&lock,0,sizeof(lock));
    lock.l_type = LOCK_MODES;
    if (fcntl(fd,F_SETLKW,&lock) == -1){// lock the file
        fcntl_syscall_error_print();
        return 1;
    }
    
    fprintf(stdout," -- File LOCK Is Realsed By Other Process !!! Conituning For The Execution...\n");

    while (token != NULL){// tokanizer for char ';'
        
        *token++ = '\0';
        counter = 0;
        sub_token = strtok(string, "/");
        
        while (sub_token != NULL){ // tokanizer for char '/'
            if (counter == 0){
                str1 = sub_token;
            }else if (counter == 1){
                str2 = sub_token;
            }else if (counter == 2){
                case_insensetive = sub_token;
            }else{
                fprintf(stderr, "Wrong String Input Format !!!\n");
                print_error_1(); // string format usage printer
                return 1;
            }
            sub_token = strtok(NULL, "/");
            counter++;
        }

        if (str1 == NULL || str2 == NULL){
            fprintf(stderr, "Not Sufficient Strings To Replace !!!\n");
            print_error_1();
            return 1;
        }

        if (case_insensetive  == NULL){ // set case sensitivity 1 or case insensitivity.
            case_insensetive_flag = 0;
        }else{
            case_insensetive_flag = 1;
        }

        if (parse_strings(str_pair_counter,str1,str2, case_insensetive_flag,fd, path) == 1){ // if some error has occured in the parse_string function, elegantly close the files and exit.
        
            if (close(fd) == -1){
                close_syscall_error_print();
                return 1;
            }
            return 1;
        }

        str_pair_counter++;
        string = token;
        str1 = str2 = case_insensetive = NULL;
        token = strchr(string, ';');
    }

    path = NULL;
    lock.l_type = F_UNLCK;    
    if (fcntl(fd,F_SETLKW,&lock) == -1){// unlock the file
        fcntl_syscall_error_print();
        if (close(fd) == -1){
            close_syscall_error_print();
            return 1;
        }
    }
    
    if (close(fd) == -1){
        close_syscall_error_print();
        return 1;
    }
    
    fprintf(stdout," -- DONE, PROGRAM EXECUTED SUCCESSFULLY :)\n");
    return 0;
}