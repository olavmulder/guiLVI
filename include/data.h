#ifndef DATA_H
#define DATA_H

#include "syncAlg.h"

extern char ownAddr_[20];

int _MakeMsgLvi(mesh_data *ret, char*msgStr, size_t len);

int HandleIncomingData(mesh_data *ret, char* buffer, size_t len);


//handling functions
mesh_data HandleSendErr(int id);
int HandleHeartBeat(mesh_data *ret, cJSON *objPtr);
int HandleClientData(mesh_data *ret, cJSON* objPtr);
int MakeMsgStringHeartbeat(char* msg, strip_t* dataStrip);

extern int MakeChangeLog(sync_data* data_to_sync, size_t len);
#endif