#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "strip.h"

void InitTimer(Node* node);
int FindTimedOutID(int id);
void SetAlive(Node*, bool isAlive);
void RestartTimer(timer_t timerid);
void HeartbeatHandler(uint8_t id, strip_t* childsStrip);

extern int MakeChangeLog(unsigned int id, int8_t voltageState, int8_t closeState, double temp,
                            bool active, bool uVolt, bool uClose, bool uTemp, bool uActive);
#endif
