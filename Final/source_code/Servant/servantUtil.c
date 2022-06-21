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
#include <dirent.h>
#include <semaphore.h>
#include "servantUtil.h"
#include "../lib/lib.h"



pid_t get_pid_from_proc_self(){
    char target[32];
    int pid;
    readlink("/proc/self",target,sizeof(target));
    sscanf(target,"%d",&pid);
    return (pid_t) pid;
}
