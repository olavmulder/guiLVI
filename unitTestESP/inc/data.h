#ifndef DATA_H
#define DATA_H



#include "libs/cJSON.h"
#include "libs/cJSON_Utils.h"
#include "heartbeat.h"

extern char* hostnameServer;
extern const char* nameGenState;
extern const char* nameVoltageState;
extern const char* nameTemp;

extern const char* nameMAC;
extern const char* nameID;
extern const char* nameCMD;
extern const char* namePort;
extern const char* nameIP;

extern const char* nameIsAlive;
extern const char* nameServerCount;
extern const char* nameServerType;
extern volatile bool coapStarted;
extern volatile bool coapInitDone;

int HandleIncomingData(char* data, double* temp_data);
mesh_data HandleIncomingCMD(/*mesh_addr_t* from,*/ char* data);


int ExtractDataFromMsg(char* msg, mesh_data* r);

int MakeMsgString(char* msg, mesh_data *r, char *ip, uint16_t *port,
                    bool gState, bool tState, bool vState);

int MakeMsgStringHeartbeat(uint8_t* msg, strip_t* data);
int MakeServerCountMsg(char* msg, size_t len, uint8_t serverCount, uint8_t serverType);
int ReceiveClient(CMD cmd, char* data);

extern void HeartbeatHandler(uint8_t id, strip_t*);
extern void RestartTimer(timer_t timerid);
#endif