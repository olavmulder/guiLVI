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
        if(monitoring_head != NULL)
        {
            if(MakeMsgStringHeartbeat(tx_buf, monitoring_head) < 0)
            {
                printf("%s, make string heartbeat error\n", __func__);
            }        
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

    pthread_create(&guiTask, NULL, gui, argv[1]);
    pthread_create(&server, NULL, ServerTask, c.ownAddr);
    pthread_create(&client, NULL, ClientTask,(void*)&c);
    pthread_create(&coap, NULL, LviTaskCOAP, c.ownAddr);
    //pthread_create(&heart, NULL, HeartbeatTask, c.ownAddr);

    #ifdef C
    pthread_join(client, NULL);
    #endif
    pthread_join(guiTask, NULL);
    pthread_join(server, NULL);
    pthread_join(coap, NULL);
    //pthread_join(heart, NULL);
    //test_is_ID_server();
    

}
void test_is_ID_server()
{
    assert(is_ID_Server(255, 0) == true);
    assert(is_ID_Server(254, 0) == true);

    assert(is_ID_Server(254, -1) == true);
    assert(is_ID_Server(255, -1) == true);
        
    assert(is_ID_Server(255, 254) == true);
    assert(is_ID_Server(255, 255) == true);
    assert(is_ID_Server(254, 254) == true);

    assert(is_ID_Server(0, 1) == false);
    printf("%s; completed\n", __func__);
}
void testDeteteStrip()
{
    static int init = 0;
    if (init == 0)
    {
        monitoring_head = malloc(sizeof(strip_t));
        monitoring_head->childArr = (Node **)malloc(sizeof(Node *));
        // monitoring_head->childArr[0] =(Node*)malloc(sizeof(Node));
        monitoring_head->lenChildArr = 0;
        init = 1;
    }
    Node a = {.id = 0, .isAlive = true};
    monitoring_head = AddNodeToStrip(monitoring_head, &a);
    Node b = {.id = 1, .isAlive = true};
    monitoring_head = AddNodeToStrip(monitoring_head, &b);
    Node c = {.id = 2, .isAlive = true};
    monitoring_head = AddNodeToStrip(monitoring_head, &c);
    printf("list len: %d\n", monitoring_head->lenChildArr);
    monitoring_head = RemoveFromStrip(monitoring_head, 1);
    monitoring_head = RemoveFromStrip(monitoring_head, 1);
    monitoring_head = AddNodeToStrip(monitoring_head, &b);
    DisplayMonitoringString();
}