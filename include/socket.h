#ifndef SOCKET_H
#define SOCKET_H
#include "data.h"

#define MAX_CLIENTS 10
#define BUF_RX_SIZE 2000
#define PORT 8080


int ServerLoopTCP_IP_MULTI(char*);
void ClientLoop(char*, char*);
int SendSync(const char *msg, size_t len);
#endif