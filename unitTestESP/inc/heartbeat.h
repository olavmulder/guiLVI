#ifndef HEARTBEAT_H
#define HEARTBEAT_H

/*#include "ethernet.h" //also data.h -> utils.h
#include "wifi.h"*/
#include "libs/strip.h"

#define MAX_SEND_FAILURE 3

//esp_timer_handle_t heartbeat_confirm_timer;
extern Node nodeHBTimer;
extern int last_layer, mesh_layer;
extern uint8_t serverTimeout;
strip_t* StartHeartbeat(strip_t *strip);
void HeartbeatHandler(uint8_t id, strip_t* childsStrip);
int SendHeartbeat();

int SendServerCountToNodes(uint8_t serverCount, uint8_t type);
int IncrementServerIpCount(bool type);
void HandleSendFailure(bool type);

void TimerHBConfirmCallback();
void TimerHBConfirmInit();
void InitTimer(Node *node, void * function);
void RestartTimer(timer_t timerid);

void SetAlive(strip_t * strip, uint8_t id, bool isAlive);
extern int SendData(char*, size_t);

extern int MakeMsgString(char* msg, mesh_data *r, char *ip, uint16_t *port,
                    bool gState, bool tState, bool vState);
#endif