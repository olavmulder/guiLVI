#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "utils.h"

extern char ip_eth[20];
extern char mac_eth[20];
extern char ip_wifi[20];
extern char mac_wifi[20];

int SendToServer();
int SendData(char* msg, size_t len);
int ReceiveData();

#endif