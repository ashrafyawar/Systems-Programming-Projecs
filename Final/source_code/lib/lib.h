#ifndef LIB_H_
#define LIB_H_

#define MESSAGE_LEN 1024
#define MODE S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH
#define CITY_LIMIT 1024

struct clientReqServantInfo{
	int flag; // 1 for client 2 for servant info
	
	//server info >>
	char serverIP[MESSAGE_LEN];
	int serverPORT;

	// servant info >>
	int servantPort;
	char servantAvailableCities[CITY_LIMIT];
	int pid;
	char startCity[100];
	char endCity[100];
	char sigInt;
	// client info >>
	char requestStr[MESSAGE_LEN];
	int threadInd;
	char transactionCount[MESSAGE_LEN];
	char transactionType[MESSAGE_LEN];
	int fromDayDate,fromMonthDate,fromYearDate;
	int toDayDate,toMonthDate,toYearDate;
	char cityName[MESSAGE_LEN];
}clientReqServantInfo;

void printMessage(char* message);
int openSocket(int socket1,char* IP,int PORT);
#endif
