#ifndef UTILS_H
#define UTILS_H
#define DEBUG

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

//#define C

/*
temerature updates every 500 ms
dropdown updtes evere 10ms
*/
#define AMOUNT_SERVER_ADDRS 2
#define LIST_SIZE 1024
#define AMOUNT_NODES 8

typedef enum{
  Err = -1, R = 0, G, Y
}ClosingState;

typedef struct __DataList
{
    int8_t voltageState;
    int8_t closeState;
    double temp;
    unsigned int id;
    bool active;
}DataList;

typedef enum _CMD{
    CMD_ERROR, CMD_INIT_VALID, CMD_INIT_INVALID, CMD_INIT_SEND,
    CMD_TO_CLIENT, CMD_TO_SERVER, CMD_HEARTBEAT, CMD_HEARTBEAT_CONFIRM, CMD_SEND_ERR, 
    CMD_SET_SERVER_NUMBER, CMD_SEND_BROADCAST, CMD_SYNC, CMD_SYNC_INIT
}CMD;


typedef struct _mesh_data{
    //mesh node info
    unsigned int id;
    char mac[20];
    char ip[20];
    uint16_t port;
    //msg type
    CMD cmd;
    //data
    int8_t voltageState;
    int8_t closeState;
    double temp;
}mesh_data;

typedef struct 
{
    int id;
    char mac[20];
    char ip[20];
    bool init;    
}init_t;

typedef struct _sync_data
{
    int id;
    int8_t voltageState;
    int8_t closeState;
    double temp;
    bool active;
    bool uVolt;
    bool uClose;
    bool uTemp;
    bool uActive;
}sync_data;
extern DataList list[LIST_SIZE];


extern char ownAddr_[20];
extern const char *SERVER_IP[AMOUNT_SERVER_ADDRS]; //server ip's,

bool is_CMD_a_Return_Msg(CMD cmd);
bool is_ID_Server(int id, int id2);
int othersID();
int myID();
#endif