/*
 * Copyright (c) 2018, 2019 Amine Ben Hassouna <amine.benhassouna@gmail.com>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without
 * limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
 * ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
/**
 * sync: TCP/IP socket connection -> in data -> HandleIncomingData(data.c)
 *                                -> out data
 * lvi communication:   a: TCP/IP (eth) 
 *                      b: COAP (wifi)
 *                         -> handling same data HandleIncomingData(data.c)
 *                         -> sending and received different                      
*/
#include "../include/gtk.h"
#include "../include/socket.h"
#include "../include/coap.h"
//change server ip to 192.168.2.103
//after that set client on a specific bind
//after that client & server test...
volatile bool hasChanged;

struct clientArg
{
    char ownAddr[20];
    char serverAddr[20];
};
void *ClientTask(void* vargp)
{ 
    //#ifdef C
    struct clientArg a = *(struct clientArg*)vargp;
    //a.ownAddr, a.serverAddr
    
    //ClientLoop(a.ownAddr, a.serverAddr );  
    //#endif 
    return NULL;
}

void *ServerTask(void *vargp)
{
    #ifndef C //(char*)vargp
    ServerLoopTCP_IP_MULTI((char*)vargp);//tcp/ip connection
    #endif
    return NULL;
}
void *LviTaskCOAP(void *vargp)
{
    //is already while loop; for connection with lvi's
    //
    //103
    //192.168.2.103
    //192.168.178.25
    //ServerLoopTCP_IP();
    
    //int res = ServerLoop((char*)vargp, "5683", "server");
    //printf("res coap %d", res);
    fflush(stdout);
    return NULL;
}

int main(int argc, char* argv[])
{
    // Unused argc, argv
    (void) argc;
    (void) argv;
    
    pthread_t client;
    pthread_t guiTask;
    pthread_t server;
    pthread_t coap;

    struct clientArg c;
    strcpy(c.ownAddr, argv[2]);
    strcpy(c.serverAddr, argv[3]);

    pthread_create(&guiTask, NULL, gui, argv[1]);
    pthread_create(&server, NULL, ServerTask, c.ownAddr);
    pthread_create(&client, NULL, ClientTask,(void*)&c);
    //pthread_create(&coap, NULL, LviTaskCOAP, c.ownAddr);
   
    pthread_join(client, NULL);
    pthread_join(guiTask, NULL);
    pthread_join(server, NULL);
    //pthread_join(coap, NULL);
}
