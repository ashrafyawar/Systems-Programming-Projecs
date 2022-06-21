#ifndef SERVER_UTIL_H_
#define SERVER_UTIL_H_

#include "../lib/lib.h"

typedef struct ARGUMENT{
    char* PORT;
    int Number_of_Threads;
}ARGUMENT;


typedef struct servantInfoStruct{
    int port;
    char cities[CITY_LIMIT];
    int pid;
}servantInfoStruct;

#endif
