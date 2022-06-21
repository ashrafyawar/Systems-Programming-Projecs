#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "helperfunctions.h"

#define BUFFER_LIMIT 5000// buffer size used to read from file.

int parse_strings(int str_pair_counter,char *str1, char *str2, int case_insensetive_flag, int fd,const char *path){

    char *bp = NULL, *result = NULL;
    int normal_string_flag = 0,found_brace_flag = 0,found_rabbit_mul_flag = 0, found_dollar_flag = 0, found_star_flag = 0;
    int limit = strlen(str1) + BUFFER_LIMIT,temp_file_fd = 0,bytesread = 0,byteswrite = 0,lseek_result = 0,do_read_write_result = 0;
    char template[] = "temp_fileXXXXXX", buf[limit], temp_str[strlen(str1)-1];
    char p1[strlen(str1)],p2[strlen(str1)],p3[strlen(str1)];

    lseek_result = lseek(fd,0,SEEK_SET);// file offset
    if (lseek_result == -1){
        lseek_syscall_error_print();
        return 1;
    }
    
    temp_file_fd = mkstemp(template);// create temporary file.
    if (temp_file_fd == -1){
        fprintf(stderr, "Couldn't Create Temporary File !!!!\n");
        mkstemp_syscall_error_print(); // possible errors for mkstemp.
        unlink(template); // unlink the temp file.
        return 1;
    }

    for (int i = 0; i < strlen(str1); ++i){temp_str[i] = '\0';}// set 

    if (!strchr(str1,'^') && !strchr(str1,'$') && !strchr(str1,'*') && !(strchr(str1,'[') || strchr(str1,']'))){ // a simple string
        strncpy(temp_str,str1,strlen(str1));
        normal_string_flag = 1;
        do_read_write_result = do_read_write(fd,str_pair_counter,temp_str,bytesread,byteswrite,limit,buf,bp,case_insensetive_flag,normal_string_flag,found_rabbit_mul_flag,found_dollar_flag,found_star_flag,found_brace_flag,str1,str2,temp_file_fd,result,template);
        
        if (do_read_write_result == 1){
            unlink(template); // unlink the temp file.
            close(temp_file_fd); // close the temp file.
            return 1;
        }
    }
    if (strchr(str1,'^')){ // if string with ^
        found_rabbit_mul_flag = 1;
        strncpy(temp_str,str1+1,strlen(str1-1));
        do_read_write_result = do_read_write(fd,str_pair_counter,temp_str,bytesread,byteswrite,limit,buf,bp,case_insensetive_flag,normal_string_flag,found_rabbit_mul_flag,found_dollar_flag,found_star_flag,found_brace_flag,str1,str2,temp_file_fd,result,template);       

        if (do_read_write_result == 1){
            unlink(template); // unlink the temp file.
            close(temp_file_fd); // close the temp file.
            return 1;
        }
    }
    if (strchr(str1,'$')){
        found_dollar_flag = 1;
        strncpy(temp_str,str1,strlen(str1)-1);
        do_read_write_result = do_read_write(fd,str_pair_counter,temp_str,bytesread,byteswrite,limit,buf,bp,case_insensetive_flag,normal_string_flag,found_rabbit_mul_flag,found_dollar_flag,found_star_flag,found_brace_flag,str1,str2,temp_file_fd,result,template);
        
        if (do_read_write_result == 1){    
            unlink(template); // unlink the temp file.
            close(temp_file_fd); // close the temp file.
            return 1;
        }
        
    }
    if (strchr(str1,'*')){
        found_star_flag = 1;
        do_read_write_result =  do_read_write(fd,str_pair_counter,temp_str,bytesread,byteswrite,limit,buf,bp,case_insensetive_flag,normal_string_flag,found_rabbit_mul_flag,found_dollar_flag,found_star_flag,found_brace_flag,str1,str2,temp_file_fd,result,template);
        
        if (do_read_write_result == 1){
            unlink(template); // unlink the temp file.
            close(temp_file_fd); // close the temp file.

            return 1;
        }
    }
    if ((strchr(str1,'[') || (strchr(str1,']')))){
        found_brace_flag = 1;
        strncpy(temp_str,str1,strlen(str1)-1);

        char ch = str1[0];
        int index = 0,k = 0;
        
        for (int i = 0; i < strlen(str1); ++i){
            p1[i] = '\0';
            p2[i] = '\0';
            p3[i] = '\0';
        }
        while (ch != '['){
            p1[k] = str1[index];
            ch = str1[++index];
            k++;
        }
        k  = 0;
        index++;
        ch = str1[index];
        while(ch !=']'){
            p2[k] = str1[index];
            ch = str1[++index];
            k++;
        }
        k = 0;
        index++;
        ch = str1[index];
        while(ch !='\0'){
            p3[k] = str1[index];
            ch = str1[++index];
            k++;
        }

        for (int i = 0; i < strlen(p2); ++i){
            normal_string_flag = 1;
            strcpy(temp_str,p1);
            strncat(temp_str,&p2[i],1);
            strcat(temp_str,p3);
            do_read_write_result = do_read_write(fd,str_pair_counter++,temp_str,bytesread,byteswrite,limit,buf,bp,case_insensetive_flag,normal_string_flag,found_rabbit_mul_flag,found_dollar_flag,found_star_flag,found_brace_flag,str1,str2,temp_file_fd,result,template);
            
            if (do_read_write_result == 1){
                unlink(template); // unlink the temp file.
                close(temp_file_fd);
                return 1;
            }
            for (int i = 0; i < strlen(str1); ++i){temp_str[i] = '\0';}
        }

    }
    for (int i = 0; i < strlen(str1); ++i){temp_str[i] = '\0';}

    unlink(template); // unlink the temp file.
    close(temp_file_fd);

    return 0;
}

int do_read_write(  int fd,int str_pair_counter, char *temp_str,int bytesread, int byteswrite, int limit, char *buf,
                    char *bp,int case_insensetive_flag,int normal_string_flag,int found_rabbit_mul_flag,int found_dollar_flag,
                    int found_star_flag,int found_brace_flag,char* str1,char* str2, int temp_file_fd,char *result,char* template){


    fprintf(stdout," -- Replacing String Pairs Of Number: %d ...\n", str_pair_counter);
    fprintf(stdout," -- Replacing String '%s' To String '%s' ...\n",temp_str,str2);
        
    for (;;){// read from original file, replace the strings and write into temp file.
        
        while((bytesread = read(fd,buf,limit)) == -1 && errno == EINTR);
        if (bytesread <= 0){ 
            read_syscall_error_print();
            break;
        }
        bp = buf;

        result = replace_strings(case_insensetive_flag,normal_string_flag,found_rabbit_mul_flag,found_dollar_flag,found_star_flag,found_brace_flag,bp,temp_str,str2);
        
        while((byteswrite = write(temp_file_fd,result,limit)) == -1 && errno == EINTR);
        free(result);            
        if (byteswrite < 0){ 
            write_syscall_error_print();
            break;
        }
        for(int i=0;i <  limit;i++){buf[i] = '\0';}// reset the buf array with nulls (to handle the overlapping at the end of the file.)

    }
    
    bytesread = 0; 
    byteswrite = 0;
    
    
    if (lseek(temp_file_fd,0,SEEK_SET) == -1){ // set fd to the beginnig of the file.
        lseek_syscall_error_print();
        return 1;
    }
    if (lseek(fd,0,SEEK_SET) == -1){//  set fd to the beginnig of the file.
        lseek_syscall_error_print();
        return 1;
    }
    
    for (;;){// read from temp file and write into original file.

        while((bytesread = read(temp_file_fd,buf,limit)) == -1 && errno == EINTR);
        if (bytesread <= 0){
            read_syscall_error_print();
            break;
        }
        bp = buf;

        char temp[strlen(bp)];
        strcpy(temp,bp);
        
        while((byteswrite = write(fd,temp,strlen(bp))) == -1 && errno == EINTR);
        if (byteswrite < 0){ 
            write_syscall_error_print();
            break;
        }
        for(int i=0;i <  limit;i++){buf[i] = '\0';}

    }

    return 0;
}


char* replace_strings(int case_insensetive_flag,int normal_string_flag,int found_rabbit_mul_flag,int found_dollar_flag,int found_star_flag,int found_brace_flag,char* sentence,char* str1, char* str2){
    char* result = NULL;
    sentence[strlen(sentence)] = '\0';
    int i = 0, counter = 0,str2_len = strlen(str2),str1_len = strlen(str1);
    
    if (sentence != NULL){
        
        if (case_insensetive_flag){ // if case insensitive then tolower the both strings to compare it.
            to_lower_string(sentence);
            to_lower_string(str1);
        }
    
        while(sentence[i] != '\0'){
            if (strstr(&sentence[i], str1) == &sentence[i]) {
                counter++;
                i += str1_len - 1;
            }
            i++;
        }
    }

    // Making new string of enough length to be stored later on in temp file and eventually on original file.
    result = (char*)malloc(i + counter * (str2_len - str1_len) + 1);
    i = 0;
    while (*sentence) {
        // compare the substring with the result
        char *temp_strstr_result = strstr(sentence, str1);
        
        if ( temp_strstr_result == sentence) {
            if (normal_string_flag == 1){ // if normal string with no regex.
                strcpy(&result[i], str2);
                i += str2_len;
                sentence += str1_len;
            
            }
            else{// if not a simple string and with some regex.
                
                if (found_rabbit_mul_flag == 1 ){

                    if (*(&sentence[temp_strstr_result - sentence-1]) == '\n' ||  *(&sentence[temp_strstr_result - sentence-1]) == '\r'){
                        
                        strcpy(&result[i], str2);
                        i += str2_len;
                        sentence += str1_len;

                    }else{

                        strcpy(&result[i], str1);
                        i += str1_len;
                        sentence += str1_len;
                    }

                }
                if (found_dollar_flag == 1){
                    if (*(&sentence[*temp_strstr_result - *sentence + str1_len]) == '\n' || *(&sentence[*temp_strstr_result - *sentence + str1_len]) == '\r'){
                        strcpy(&result[i], str2);
                        i += str2_len;
                        sentence += str1_len;
                    }else{
                        strcpy(&result[i], str1);
                        i += str1_len;
                        sentence += str1_len;
                    }
                }
            }
        }
        else{
            result[i++] = *sentence++;
        }
    }
    result[i] = '\0';
    return result;
}

void open_syscall_error_print(){
    
    if (errno == EACCES){
        fprintf(stderr,"The requested access to the file is not allowed!!!\n");
    }else if (errno == EDQUOT){
        fprintf(stderr,"Where O_CREAT is specified, the file does not exist!!!\n");
    }else if (errno == EEXIST){
        fprintf(stderr,"Pathname already exists and O_CREAT and O_EXCL were used!!!\n");
    }else if (errno == EFAULT){
        fprintf(stderr,"Pathname points outside your accessible address space!!!\n");
    }else if (errno == EFBIG){
        fprintf(stderr,"See EOVERFLOW!!!\n");
    }else if (errno == EINTR){
        fprintf(stderr,"While  blocked  waiting  to  complete  an  open  of a slow device!!!\n");
    }else if (errno == EINVAL){
        fprintf(stderr,"Invalid value in flags!!!\n");
    }else if (errno == ENOTDIR){
        fprintf(stderr,"Pathname  is  a relative pathname and dirfd is a file descriptor referring to a file other than a directory!!!\n");
    }
}

void close_syscall_error_print(){

    if (errno == EBADF){
        fprintf(stderr,"fd isn't a valid open file descriptor!!!\n");
    }else if (errno == EINTR){
        fprintf(stderr,"The close() call was interrupted by a signal!!!\n");
    }else if (errno == EIO){
        fprintf(stderr,"An I/O error occurred!!!\n");
    }else if (errno == ENOSPC){
        fprintf(stderr,"On NFS, these errors are not normally reported against the first write which exceeds the available storage space, but instead against a subsequent write(2), fsync(2), or close().!!!\n");
    }

    
}
void print_error_1(){

    fprintf(stderr, "Usage: '/str1/str2/' (replace all occurrences of str1 with str2)\n");
    fprintf(stderr, "OR '/str1/str2/i' (perform the same as above, but will be case insensitive) \n");
    fprintf(stderr, "OR '/str1/str2/i;/str3/str4/' (replace multiple replacement operations)\n");
    fprintf(stderr, "OR '/[zs]tr1/str2/' (replace multiple character matching; e.g. this will match both ztr1 and str1)\n");
    fprintf(stderr, "OR '/^str1/str2/' (replace matching at line starts; e.g. this will only match lines starting with str1)\n");
    fprintf(stderr, "OR '/str1$/str2/' (replace matching at line ends; e.g. this will only match lines ending with str1)\n");
    fprintf(stderr, "OR '/st*r1/str2/' (replace zero or more repetitions of characters; e.g. this will match sr1, str1, sttr1, sttttr1, etc)\n");
    fprintf(stderr, "OR '/^Window[sz]*/Linux/i;/close[dD]$/open/' (replace arbitrary combinations of the above methods)\n");
}

void fcntl_syscall_error_print(){
    
    if (errno == EACCES){
        fprintf(stderr,"The requested access to the file is not allowed!!!\n");
    }else if (errno == EAGAIN){
        fprintf(stderr,"The  operation  is  prohibited  because  the  file has been memory-mapped by another process!!!\n");
    }else if (errno == EBADF){
        fprintf(stderr,"fd is not an open file descriptor!!!\n");
    }else if (errno == EBUSY){
        fprintf(stderr,"CMD is F_ADD_SEALS, arg includes F_SEAL_WRITE, and there exists a  writable,  shared mapping on the file referred to by fd.!!!\n");
    }else if (errno == EDEADLK){
        fprintf(stderr,"It was detected that the specified F_SETLKW command would cause a deadlock!!!\n");
    }else if (errno == EFAULT){
        fprintf(stderr,"Lock is outside your accessible address space!!!\n");
    }else if (errno == EINTR){
        fprintf(stderr,"CMD is  F_SETLKW or F_OFD_SETLKW and the operation was interrupted by a signal; see signal(7).!!!\n");
    }else if (errno == EINVAL){
        fprintf(stderr,"The value specified in cmd is not recognized by this kernel!!!\n");
    }else if (errno == EMFILE){
        fprintf(stderr,"CMD  is F_DUPFD and the per-process limit on the number of open file descriptors has been reached.!!!\n");
    }else if (errno == EAGAIN){
        fprintf(stderr,"The  operation  is  prohibited  because  the  file has been memory-mapped by another process!!!\n");
    }else if (errno == ENOLCK){
        fprintf(stderr,"Too many segment locks open, lock table is full, or a remote locking protocol failed (e.g., locking over NFS)!!!\n");
    }else if (errno == ENOTDIR){
        fprintf(stderr,"F_NOTIFY was specified in cmd, but fd does not refer to a directory.!!!\n");
    }
    else if (errno == EPERM){
        printf("Attempted to clear the O_APPEND flag on a file that has  the  append-only  attribute set!!!\n");
    }
}

void lseek_syscall_error_print(){

    if (errno == EBADF){
        fprintf(stderr,"fd is not an open file descriptor !!!\n");
    }else if (errno == EINVAL){
        fprintf(stderr,"Whence is not valid.  Or: the resulting file offset would be negative, or beyond the end of a seekable device.!!!\n");
    }else if (errno == ENXIO){
        fprintf(stderr,"Whence is SEEK_DATA or SEEK_HOLE, and the file offset is beyond the end of the file.!!!\n");
    }else if (errno == EOVERFLOW){
        fprintf(stderr,"The resulting file offset cannot be represented in an off_t.!!!\n");
    }else if (errno == ESPIPE){
        fprintf(stderr,"fd is associated with a pipe, socket, or FIFO.!!!\n");
    }

}

void mkstemp_syscall_error_print(){
    if (errno == EEXIST){
        fprintf(stderr,"Could not create a unique temporary filename.  Now the contents of template are  un‐ defined. !!!\n");
    }else if (errno == EINVAL){
        fprintf(stderr,"For  mkstemp()  and mkostemp(): The last six characters of template were not XXXXXX now template is unchanged.!!!\n");
    }
}

void unlink_syscall_error_print(){

    if (errno == EACCES){
        fprintf(stderr,"Write  access  to the directory containing pathname is not allowed for the process's effective UID, or one of the directories in pathname did not  allow  search  permis‐ sion.  (See also path_resolution(7).) !!!\n");
    }else if (errno == EBUSY){
        fprintf(stderr,"The file  pathname cannot be unlinked because it is being used by the system or another process; for example, it is a mount point or the NFS client  software  created it to represent an active but otherwise nameless inode ('NFS silly renamed').!!!\n");
    }else if (errno == EFAULT){
        fprintf(stderr,"Pathname points outside your accessible address space!!!\n");
    }else if (errno == EIO){
        fprintf(stderr,"An I/O error occurred!!!\n");
    }else if (errno == EISDIR){
        fprintf(stderr,"pathname  refers  to  a  directory.   (This is the non-POSIX value returned by Linux since 2.1.132.)!!!\n");
    }else if (errno == ELOOP){
        fprintf(stderr,"Too many symbolic links were encountered in translating pathname!!!\n");
    }else if (errno == ENAMETOOLONG){
        fprintf(stderr,"Pathname was too long!!!\n");
    }else if (errno == ENOENT){
        fprintf(stderr,"A component in pathname does not exist or is a dangling symbolic link,  or  pathname is empty!!!\n");
    }else if (errno == ENOMEM){
        fprintf(stderr,"Insufficient kernel memory was available!!!\n");
    }else if (errno == ENOTDIR){
        fprintf(stderr,"A component used as a directory in pathname is not, in fact, a directory!!!\n");
    }else if (errno == EBADF){
        fprintf(stderr,"dirfd is not a valid file descriptor!!!\n");
    }else if (errno == EINVAL){
        fprintf(stderr,"An invalid flag value was specified in flags!!!\n");
    }else if (errno == EISDIR){
        fprintf(stderr,"pathname refers to a directory, and AT_REMOVEDIR was not specified in flags!!!\n");
    }else if (errno == ENOTDIR){
        fprintf(stderr,"pathname is relative and dirfd is a file descriptor referring to a file other than a directory.!!!\n");
    }
}


void read_syscall_error_print(){
    
    if (errno == EAGAIN){
        fprintf(stderr,"The file descriptor fd refers to a file other than a socket and has been marked non‐blocking!!!\n");
    }else if (errno == EBADF){
        fprintf(stderr,"fd is not a valid file descriptor or is not open for reading!!!\n");
    }else if (errno == EFAULT){
        fprintf(stderr,"buf is outside your accessible address space!!!\n");
    }else if (errno == EINTR){
        fprintf(stderr,"The call was interrupted by a signal before any data was read!!!\n");
    }else if (errno == EINVAL){
        fprintf(stderr,"fd is attached to an object which is unsuitable for reading; or the file was  opened with the O_DIRECT flag, and either the address specified in buf, the value specified in count, or the file offset is not suitably aligned!!!\n");
    }else if (errno == EIO){
        fprintf(stderr,"I/O error!!!\n");
    }else if (errno == EISDIR){
        fprintf(stderr,"fd refers to a directory!!!\n");
    }
}

void write_syscall_error_print(){
    
    if (errno == EAGAIN){
        fprintf(stderr,"The file descriptor fd refers to a file other than a socket and has been marked non‐blocking!!!\n");
    }else if (errno == EBADF){
        fprintf(stderr,"fd is not a valid file descriptor or is not open for reading!!!\n");
    }else if (errno == EDESTADDRREQ){
        fprintf(stderr,"fd  refers to a datagram socket for which a peer address has not been set using connect(2)!!!\n");
    }else if (errno == EDQUOT){
        fprintf(stderr,"The user's quota of disk blocks on the filesystem containing the file referred to by fd has been exhausted!!!\n");
    }else if (errno == EFAULT){
        fprintf(stderr,"buf is outside your accessible address space!!!\n");
    }else if (errno == EFBIG){
        fprintf(stderr,"An  attempt was made to write a file that exceeds the implementation-defined maximum file size or the process's file size limit, or to write at a position past the maximum allowed offset.!!!\n");
    }else if (errno == EINTR){
        fprintf(stderr,"The call was interrupted by a signal before any data was written!!!\n");
    }else if (errno == EINVAL){
        fprintf(stderr,"fd  is attached to an object which is unsuitable for writing!!!\n");
    }else if (errno == EIO){
        fprintf(stderr,"A  low-level I/O error occurred while modifying the inode!!!\n");
    }else if (errno == ENOSPC){
        fprintf(stderr,"The device containing the file referred to by fd has no room for the data!!!\n");
    }else if (errno == EPERM){
        fprintf(stderr,"The operation was prevented by a file seal!!!\n");
    }else if (errno == EPIPE){
        fprintf(stderr,"fd is connected to a pipe or socket whose reading end is closed!!!\n");
    }
}

char *to_lower_string(char *str){
  unsigned char *pointer = (unsigned char *)str;
  while (*pointer) {
    *pointer = to_lower((unsigned char)*pointer);
    pointer++;
  }
  return str;
}

int to_lower(int character){ // custom to lower function 

    if(character >= 'A' && character <= 'Z'){
        return (character + 'a' - 'A');

    }else{
        return(character);

    }

}