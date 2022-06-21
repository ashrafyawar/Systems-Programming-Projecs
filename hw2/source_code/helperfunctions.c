#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "helperfunctions.h"
#include<math.h>
    
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

void close_syscall_error_print(){

    if (errno == EBADF){
        perror("fd isn't a valid open file descriptor!!!\n");
    }else if (errno == EINTR){
        perror("The close() call was interrupted by a signal!!!\n");
    }else if (errno == EIO){
        perror("An I/O error occurred!!!\n");
    }else if (errno == ENOSPC){
        perror("On NFS, these errors are not normally reported against the first write which exceeds the available storage space, but instead against a subsequent write(2), fsync(2), or close().!!!\n");
    }

    
}

void fcntl_syscall_error_print(){
    
    if (errno == EACCES){
        perror("The requested access to the file is not allowed!!!\n");
    }else if (errno == EAGAIN){
        perror("The  operation  is  prohibited  because  the  file has been memory-mapped by another process!!!\n");
    }else if (errno == EBADF){
        perror("fd is not an open file descriptor!!!\n");
    }else if (errno == EBUSY){
        perror("CMD is F_ADD_SEALS, arg includes F_SEAL_WRITE, and there exists a  writable,  shared mapping on the file referred to by fd.!!!\n");
    }else if (errno == EDEADLK){
        perror("It was detected that the specified F_SETLKW command would cause a deadlock!!!\n");
    }else if (errno == EFAULT){
        perror("Lock is outside your accessible address space!!!\n");
    }else if (errno == EINTR){
        perror("CMD is  F_SETLKW or F_OFD_SETLKW and the operation was interrupted by a signal; see signal(7).!!!\n");
    }else if (errno == EINVAL){
        perror("The value specified in cmd is not recognized by this kernel!!!\n");
    }else if (errno == EMFILE){
        perror("CMD  is F_DUPFD and the per-process limit on the number of open file descriptors has been reached.!!!\n");
    }else if (errno == EAGAIN){
        perror("The  operation  is  prohibited  because  the  file has been memory-mapped by another process!!!\n");
    }else if (errno == ENOLCK){
        perror("Too many segment locks open, lock table is full, or a remote locking protocol failed (e.g., locking over NFS)!!!\n");
    }else if (errno == ENOTDIR){
        perror("F_NOTIFY was specified in cmd, but fd does not refer to a directory.!!!\n");
    }
    else if (errno == EPERM){
        printf("Attempted to clear the O_APPEND flag on a file that has  the  append-only  attribute set!!!\n");
    }
}

void lseek_syscall_error_print(){

    if (errno == EBADF){
        perror("fd is not an open file descriptor !!!\n");
    }else if (errno == EINVAL){
        perror("Whence is not valid.  Or: the resulting file offset would be negative, or beyond the end of a seekable device.!!!\n");
    }else if (errno == ENXIO){
        perror("Whence is SEEK_DATA or SEEK_HOLE, and the file offset is beyond the end of the file.!!!\n");
    }else if (errno == EOVERFLOW){
        perror("The resulting file offset cannot be represented in an off_t.!!!\n");
    }else if (errno == ESPIPE){
        perror("fd is associated with a pipe, socket, or FIFO.!!!\n");
    }

}

void mkstemp_syscall_error_print(){
    if (errno == EEXIST){
        perror("Could not create a unique temporary filename.  Now the contents of template are  un‐ defined. !!!\n");
    }else if (errno == EINVAL){
        perror("For  mkstemp()  and mkostemp(): The last six characters of template were not XXXXXX now template is unchanged.!!!\n");
    }
}

void unlink_syscall_error_print(){

    if (errno == EACCES){
        perror("Write  access  to the directory containing pathname is not allowed for the process's effective UID, or one of the directories in pathname did not  allow  search  permis‐ sion.  (See also path_resolution(7).) !!!\n");
    }else if (errno == EBUSY){
        perror("The file  pathname cannot be unlinked because it is being used by the system or another process; for example, it is a mount point or the NFS client  software  created it to represent an active but otherwise nameless inode ('NFS silly renamed').!!!\n");
    }else if (errno == EFAULT){
        perror("Pathname points outside your accessible address space!!!\n");
    }else if (errno == EIO){
        perror("An I/O error occurred!!!\n");
    }else if (errno == EISDIR){
        perror("pathname  refers  to  a  directory.   (This is the non-POSIX value returned by Linux since 2.1.132.)!!!\n");
    }else if (errno == ELOOP){
        perror("Too many symbolic links were encountered in translating pathname!!!\n");
    }else if (errno == ENAMETOOLONG){
        perror("Pathname was too long!!!\n");
    }else if (errno == ENOENT){
        perror("A component in pathname does not exist or is a dangling symbolic link,  or  pathname is empty!!!\n");
    }else if (errno == ENOMEM){
        perror("Insufficient kernel memory was available!!!\n");
    }else if (errno == ENOTDIR){
        perror("A component used as a directory in pathname is not, in fact, a directory!!!\n");
    }else if (errno == EBADF){
        perror("dirfd is not a valid file descriptor!!!\n");
    }else if (errno == EINVAL){
        perror("An invalid flag value was specified in flags!!!\n");
    }else if (errno == EISDIR){
        perror("pathname refers to a directory, and AT_REMOVEDIR was not specified in flags!!!\n");
    }else if (errno == ENOTDIR){
        perror("pathname is relative and dirfd is a file descriptor referring to a file other than a directory.!!!\n");
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

void write_syscall_error_print(){
    
    if (errno == EAGAIN){
        perror("The file descriptor fd refers to a file other than a socket and has been marked non‐blocking!!!\n");
    }else if (errno == EBADF){
        perror("fd is not a valid file descriptor or is not open for reading!!!\n");
    }else if (errno == EDESTADDRREQ){
        perror("fd  refers to a datagram socket for which a peer address has not been set using connect(2)!!!\n");
    }else if (errno == EDQUOT){
        perror("The user's quota of disk blocks on the filesystem containing the file referred to by fd has been exhausted!!!\n");
    }else if (errno == EFAULT){
        perror("buf is outside your accessible address space!!!\n");
    }else if (errno == EFBIG){
        perror("An  attempt was made to write a file that exceeds the implementation-defined maximum file size or the process's file size limit, or to write at a position past the maximum allowed offset.!!!\n");
    }else if (errno == EINTR){
        perror("The call was interrupted by a signal before any data was written!!!\n");
    }else if (errno == EINVAL){
        perror("fd  is attached to an object which is unsuitable for writing!!!\n");
    }else if (errno == EIO){
        perror("A  low-level I/O error occurred while modifying the inode!!!\n");
    }else if (errno == ENOSPC){
        perror("The device containing the file referred to by fd has no room for the data!!!\n");
    }else if (errno == EPERM){
        perror("The operation was prevented by a file seal!!!\n");
    }else if (errno == EPIPE){
        perror("fd is connected to a pipe or socket whose reading end is closed!!!\n");
    }
}


float frobeniusNorm(float matrix[3][3]){
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            sum += power(matrix[i][j], 2);
        }
    }
 
    float result = sqrt(sum);
    return result;
}

int power (int x, int y){  
    int power = 1, i; 
    for (i = 1; i <= y; ++i){  
        power = power * x;  
          
    }  
    return power;  
}