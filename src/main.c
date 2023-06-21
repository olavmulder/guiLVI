#include "../include/gtk.h"
#include "../include/socket.h"
#include "../include/coap.h"

volatile bool hasChanged;

const char *SERVER_IP[AMOUNT_SERVER_ADDRS] = {"192.168.2.109","192.168.2.106"};//, "192.168.178.25", }; 
struct clientArg
{
    char ownAddr[20];
    char serverAddr[20];
};

void test_is_ID_server();
void test_heartbeat_init();
void test_my_id();

void *ClientTask(void* vargp)
{ 
    #ifdef C
    struct clientArg a = *(struct clientArg*)vargp;
    //a.ownAddr, a.serverAddr
    ClientLoop(a.ownAddr, a.serverAddr );  
    #endif 
    return NULL;
}

void *HeartbeatTask(void *vargp)
{
    char tx_buf[2000];
    fflush(stdout);
    while(1)
    {
        sleep(5);
        if(isHeartbeatInit())
        {
            if(MakeMsgStringHeartbeat(tx_buf, NULL) < 0)
            {
                printf("%s, make string heartbeat error\n", __func__);
            }    
            //printf("try to send hb task: %s\n", tx_buf);   
            int res = SendSync(tx_buf, strlen(tx_buf));
            if(res <= 0)
            {
                printf("%s; send error\n", __func__);
            }
            fflush(stdout);
        }
    }
}
void *ServerTask(void *vargp)
{
    ServerLoopTCP_IP_MULTI((char*)vargp);//tcp/ip connection
    return NULL;
}
void *LviTaskCOAP(void *vargp)
{
    int res = ServerLoop((char*)vargp, "5683", "server");
    while(1){
        printf("serloop exited ...\n");
        fflush(stdout);
        sleep(5);
    }
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
    pthread_t heart;
    struct clientArg c;
    strcpy(c.ownAddr, argv[2]);
    strcpy(c.serverAddr, argv[3]);
    
    HeartbeatInit();

    pthread_create(&guiTask, NULL, gui, argv[1]);
    pthread_create(&server, NULL, ServerTask, c.ownAddr);
    pthread_create(&client, NULL, ClientTask,(void*)&c);
    pthread_create(&coap, NULL, LviTaskCOAP, c.ownAddr);
    pthread_create(&heart, NULL, HeartbeatTask, c.ownAddr);

    #ifdef C
    pthread_join(client, NULL);
    #endif
    pthread_join(guiTask, NULL);
    pthread_join(server, NULL);
    pthread_join(coap, NULL);
    pthread_join(heart, NULL);
    //test_my_id();
    //test_is_ID_server();
    //test_heartbeat_init();
}


