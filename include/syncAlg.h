#ifndef SYNC_ALG_H

#define SYNC_ALG_H

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <coap3/coap.h>

#include "cJSON.h"
#include "heartbeat.h"

#define ASK_COPY_ID 2000
#define RECEIVE_COPY_ID 2001


extern uint8_t clientId;

extern const char *nameVoltageState;
extern const char *nameCloseState;
extern const char *nameTemp ;
extern const char *nameCounter;
extern const char *nameClientID;
extern const char *nameID;
extern const char *nameType;
extern const char *nameActive ;
extern const char *nameArray ;
extern const char *nameMAC;
extern const char *nameIP;
extern const char *namePort ;
extern const char *nameCMD ;
extern const char *nameACK ;

extern volatile bool hasChanged;
extern volatile bool updateFlag;

extern DataList list[LIST_SIZE];
extern bool firstTimeCallBack[AMOUNT_NODES];
extern unsigned int highestID;
extern uint64_t syncCounter;

extern volatile bool canvasInitDone; //gui init
extern volatile bool isCompleted;


int _cJSON_ErrorHandling(cJSON *obj);

int MakeChangeLog(sync_data *data_to_sync, size_t len);


char* SendCopyData();
int ReceiveCopyData(cJSON*, cJSON*);
int AskCopyData();
int _ChangeData(DataList*, bool uVolt, bool uClose,
                bool uTemp, bool uActive);
int ReadSyncCallBack(const char* msg, size_t len);
int InitMsg();
#ifdef DEBUG
void PrintList(FILE*);
#endif

extern int SendSync(const char *msg, size_t len);

extern int FindID(int id);

#endif