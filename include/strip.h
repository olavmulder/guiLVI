#ifndef STRIP_H
#define STRIP_H

#include "utils.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

typedef struct _node_t
{
   uint8_t id;
   char ip_wifi[20];
   char mac_wifi[20];
   uint16_t port;
   bool isAlive;
   timer_t timerid;
   uint8_t timeouts;
}Node;

typedef struct _strip_t
{
   Node **childArr;
   uint8_t lenChildArr;
 
}strip_t;

extern strip_t *monitoring_head;

void UpdateDependancy(strip_t* root);
Node* NewNode(int id, char* ip, char *mac, uint16_t port, bool isAlive);//data
strip_t* AddNodeToStrip(strip_t* strip, Node* n);
strip_t* RemoveFromStrip(strip_t* strip, int id);

int FindID(int id);
void DisplayMonitoringString();


extern void InitTimer(Node* node);
extern void TimerCallback(void *id);

#endif
