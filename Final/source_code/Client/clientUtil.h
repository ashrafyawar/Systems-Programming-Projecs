#ifndef CLIENT_UTIL_H_
#define CLIENT_UTIL_H_
#define MODE S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH

#include "../lib/lib.h"


struct clientResponse{
	int totalTransactionCount;
};
#endif
