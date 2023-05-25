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
}Node;

typedef struct _strip_t
{
   Node **childArr;//was idArr
   uint8_t lenChildArr;
   /*uint8_t num_childs;
   _node_t **childs;*/
}strip_t;

extern strip_t *monitoring_head;

void UpdateDependancy(strip_t* root);
Node* NewNode(int id, char* ip, char *mac, uint16_t port, bool isAlive);//data
strip_t* AddNodeToStrip(strip_t* strip, Node* n);
void DisplayMonitoringString();
/*void FindAllChildren(Node *node);
int MaxDepth(Node *root);
void UpdateDependancy(Node** root, int totalNodes);*/

extern void InitTimer(Node* node);
extern void TimerCallback(void *id);
#endif
