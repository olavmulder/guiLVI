#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "strip.h"

bool isHeartbeatInit();
bool HeartbeatInit();
void InitTimer(Node* node);
int FindTimedOutID(int id);
void SetAlive(Node*, bool isAlive);
void RestartTimer(timer_t timerid);
void HeartbeatHandler(uint8_t id, strip_t* childsStrip);
int MakeMsgStringHeartbeatSendThrough(char *msg, strip_t *dataStrip);
extern int MakeChangeLog(sync_data *data_to_sync, size_t len);

extern void CloseSyncSocket();
#endif
