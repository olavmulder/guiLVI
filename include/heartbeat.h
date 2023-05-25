#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "strip.h"

void InitTimer(Node* node);
int FindTimedOutID(int id);
void SetAlive(Node*, bool isAlive);
void RestartTimer(timer_t timerid);
#endif
