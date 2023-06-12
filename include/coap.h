#ifndef COAP_H
#define COAP_H

#include "data.h"

#define SERVER_NAME "server"



extern char* syncMsg;
extern size_t syncMsgLen;
//coap
int ServerLoop(char* ipAddr, char* port, char* serverName);

uint8_t GetClientID();

int _MakeMsgLvi(mesh_data *ret, char*msgStr, size_t len);
mesh_data _ReceiveFromLvi(const char* buf, size_t len);
void _CheckIDExist(mesh_data* ret);
int _CoapResolveAddress(const char *host, const char *service, coap_address_t *dst);

#endif