#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include<stdlib.h>
#include <signal.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define CLIENT_FIFO_TEMPLATE "response_%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)
#define BUFFER_LIMIT 1
#define BUFFER_SIZE 300000

void fcntl_syscall_error_print();
void open_syscall_error_print();
void close_syscall_error_print();
void lseek_syscall_error_print();
void mkstemp_syscall_error_print();
void unlink_syscall_error_print();
void read_syscall_error_print();
void write_syscall_error_print();

static char clientFifo[CLIENT_FIFO_NAME_LEN];

static void removeFifo(void){
    if (unlink(clientFifo)){
        perror("unlink: ");
        unlink_syscall_error_print();
    }
}

struct request{
    pid_t pid;
    int matrixLen;
    char payLoad[BUFFER_SIZE];
};
struct response{
    int isInvertable;
};

sig_atomic_t sigintcaught = 0;

void siginthandler(){
    int esaved = errno;
    sigintcaught = 1;
    errno = esaved;
}

int main(int argc, char **argv){
    
    // int printBufferPointerLen = 1000;

    char* serverFifoPath = NULL;
    char* dataFilePath = NULL;
    char buf[BUFFER_LIMIT];
    // char* printBufferPointer = (char*) malloc(printBufferPointerLen * sizeof(char));

    int serverFd = 0, clientFd = 0,bytesread = 0,dataFilePathFd = 0;
    struct request req;
    struct response resp;
    struct sigaction newact;
    time_t t;   // not a primitive datatype
    time(&t);
    
    newact.sa_handler = &siginthandler; /* set the new handler */
    newact.sa_flags = 0;
    sigaction(SIGINT,&newact,NULL);
    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT,&newact, NULL) == -1)){
        perror("Failed to install SIGINT signal handler");
        _exit(EXIT_FAILURE);
    }

    if (sigintcaught == 1){
        _exit(EXIT_FAILURE);
    }
 
    // check if the user has entered sufficient arguments.
    if (argc < 5){
        perror("No Sufficient Parameters To Execute The Edit !!!\n");
        perror("Usage: vg ./client -s pathToServerFifo -o pathToDataFile\n");
        return 1;
    }else if (argc > 5){
        perror("Too Much Parameters To Execute The Edit !!!\n");
        perror("Usage: vg ./client -s pathToServerFifo -o pathToDataFile\n");
        return 1;

    }
    
    //file paths read from terminal
    serverFifoPath = argv[2];
    dataFilePath = argv[4];
    dataFilePathFd = open(dataFilePath,O_RDONLY);
    
    if (dataFilePathFd == -1){
        perror("dataFilePathFd");
        close(dataFilePathFd);
        _exit(EXIT_FAILURE);
    }
    req.matrixLen = 0;
    for (;;){// read matrix from the file
        while((bytesread = read(dataFilePathFd,buf,BUFFER_LIMIT)) == -1 && errno == EINTR);
        if (bytesread <= 0){
            read_syscall_error_print();
            break;
        }
        if (buf[0] == '\n'){
            req.matrixLen++;
            buf[0] = ',';
        }
        strcat(req.payLoad,buf);
        for (int i = 0; i < BUFFER_LIMIT; ++i){ buf[i] = '\0';}
    }
    req.matrixLen++;

    if (sigintcaught == 1){
        close(dataFilePathFd);
        _exit(EXIT_FAILURE);
    }
    umask(0);
    snprintf(clientFifo,CLIENT_FIFO_NAME_LEN,CLIENT_FIFO_TEMPLATE, (long) getpid());
    if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){
        perror("mkfifo: ");
        close(dataFilePathFd);
        _exit(EXIT_FAILURE);
    }
    
    if (atexit(removeFifo) != 0){
        perror("atexit:");
        close(dataFilePathFd);
        _exit(EXIT_FAILURE);
    }
    
    req.pid = getpid();
    serverFd = open(serverFifoPath,O_WRONLY);

    if (serverFd == -1){
        perror("SERVER_FIFO");
        close(dataFilePathFd);
        close(serverFd);
        _exit(EXIT_FAILURE);

    }
    fprintf(stdout,"[%.19s] Client PID#%d (%s) is submitting a %dx%d matrix\n",ctime(&t),req.pid,dataFilePath,req.matrixLen,req.matrixLen);
    if (write(serverFd,&req,sizeof(struct request)) != sizeof(struct request)){
        perror("serverFd");
        close(dataFilePathFd);
        close(serverFd);
        _exit(EXIT_FAILURE);
    }

    clientFd = open(clientFifo,O_RDONLY);
    if (clientFd == -1){
        perror("clientFd");
        close(dataFilePathFd);
        close(serverFd);
        close(clientFd);
        _exit(EXIT_FAILURE);

    }

    if (read(clientFd,&resp,sizeof(struct response)) != sizeof(struct response)) {
        perror("Can't Read Response from server");
        close(dataFilePathFd);
        close(serverFd);
        close(clientFd);
        _exit(EXIT_FAILURE);
    }

    if (sigintcaught == 1){
        close(dataFilePathFd);
        close(serverFd);
        close(clientFd);
        _exit(EXIT_FAILURE);
    }
    if (resp.isInvertable){
        fprintf(stdout, "[%.19s] Client PID#%d: the matrix is invertable, total time %f seconds, goodbye.\n",ctime(&t),req.pid,0.4);

    }else{
        fprintf(stdout, "[%.19s] Client PID#%d: the matrix is not invertable, total time %f seconds, goodbye.\n",ctime(&t),req.pid,0.4);
    }
    close(clientFd);
    close(serverFd);
    close(dataFilePathFd);
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