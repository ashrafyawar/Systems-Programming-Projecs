#ifndef SERVANT_UTIL_H_
#define SERVANT_UTIL_H_

#include "../lib/lib.h"

#define FILE_COUNT 10
#define TRANSACTION_COUNT 10

struct transaction{
    char transactionAsString[MESSAGE_LEN];
    int id;
    char transactionType[MESSAGE_LEN];
    char transactionLocation[MESSAGE_LEN];
    int serfaceSquare;
    int price;
};

struct file{
    char fileName[MESSAGE_LEN];
    struct  transaction transactionArr [TRANSACTION_COUNT];
    int fileCount;
    int day;
    int month;
    int year;
};

struct city{
    char cityName[MESSAGE_LEN];
    struct file fileArr[FILE_COUNT];
    int startInd;
    int endInd;
    struct city *next;
};
struct servantInfo{
    int servantPort;
};

struct serverResponse{
    int totalTransactionCount;
};


pid_t get_pid_from_proc_self();
#endif
