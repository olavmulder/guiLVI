//#include "inc/communication.h"
#include "inc/data.h"
#include "inc/config.h"
#include "inc/utils.h"
volatile ClosingState curGeneralState = Err;
volatile VoltageFreeState curVoltageState;
double curMeasuredTemp;
double curReceivedTemp;
volatile CommunicationState comState = COMMUNICATION_NONE;
uint8_t currentServerIPCount_WIFI;
int8_t currentServerIPCount_ETH;
//wifi/eth connection variables
volatile bool initSuccessfull = false;
volatile bool is_mesh_connected = false;
volatile bool is_eth_connected = false;

volatile bool gotIPAddress = false;
volatile bool heartbeatEnable = false;

char idName[5];
//recieve eth
int receive(mesh_data *r, char* data)
{
    int ret = 0;
    *r =  HandleIncomingCMD(data);
    fflush(stdout);
    if(r->cmd == CMD_ERROR)
        ret = -1;
    else if(strcmp("aa:bb:cc", r->mac) == 0)
        ret = ReceiveClient(r->cmd, data);
    else
        ret = -3;
    
    
    return ret;
}
int ReceiveWiFiMeshLeaf(mesh_data *r, uint8_t* data)
{
    char mac_wifi[] = "aa:bb:cc"; 
    //receive own data
    int ret = -1;
    if(strcmp(r->mac, mac_wifi) == 0)
    {
        ret = ReceiveClient(r->cmd, (char*)data);
    }
    return ret;
}
/**
 * @brief 
 * 
 * @param from mesh_addr where the message came from
 * @param r    mesh_data pointer to get get data and handle it
 * @param data msg string to handle the incoming data
 * @return int -3 on invalid cmd, -2 taken semaphore, -1 failure, 0 success
 */
int ReceiveWiFiMeshRoot(mesh_data *r, uint8_t* data)
{
    if(r->cmd == CMD_TO_SERVER || r->cmd == CMD_INIT_SEND || r->cmd == CMD_SET_ERR)
    {
        char ip[LENGTH_IP];
        snprintf(ip, LENGTH_IP,"%s", "192.168.178.2");
        //ESP_LOGI(TAG_WIFI,"\nrx root%s", (char*)data);
        uint16_t port = 1800;
        //only add temperature, because server only needs temp
        
        if(HandleIncomingData((char*)data, &r->temp) != 0)
        {
           //ESP_LOGW(TAG_WIFI,"handle incoming data error");
            return -1;
        }
        char buf[500];
        if(MakeMsgString(buf, r, ip,&port,false,true,false ) != 0)
        {
            //ESP_LOGW(TAG_WIFI,"MakeMsgString error");
            return -1;
        }
        return 0;
        //return SendWiFiMeshRoot((uint8_t*)buf, strlen(buf));   
        /*if(res == 2)
        {
            do{
                res = SendWiFiMeshRoot((uint8_t*)buf, strlen(buf));
            }while(res == 2);
        }*/
    }
    return -3;

}
int ReceiveWiFiMesh(char* buf, size_t len, bool is_root)
{
    #define RX_SIZE 1500
    uint8_t rx_buf[RX_SIZE] = { 0, };
    memcpy(rx_buf, buf, len);
    int flag = 0;    
    int res = -1;
    bool bufIsFull=false;
    mesh_data r;
    if(strlen((char*)rx_buf) != 0)bufIsFull = true;
    while(bufIsFull)
    {
        //ESP_LOGI(TAG_WIFI, "%s, RECEIVE: %s", __func__, data_rx.data);
        r = HandleIncomingCMD((char*)rx_buf);
        //if it was CMD_HEARTBEAT it is already handled in last function,
        //so return 0
        if(r.cmd == CMD_ERROR)return -1;
        if(r.cmd == CMD_HEARTBEAT)return 0;
        if(is_root)
        {
            res = ReceiveWiFiMeshRoot(&r, rx_buf);
            if(res < 0)
            {
                //ESP_LOGW(TAG_WIFI, "%s: res: %d", __func__, res);
            }
        }
        else
        {
            res = ReceiveWiFiMeshLeaf(&r, rx_buf);
            if(res < 0)
            {
                //ESP_LOGW(TAG_WIFI, "%s: res: %d", __func__, res);
            }
        }
        //handle buffer
        size_t len = strlen((char*)rx_buf);
        memcpy(rx_buf, rx_buf+len, RX_SIZE-len);
        if(strlen((char*)rx_buf) > 0)bufIsFull = true;
        else bufIsFull = false;
    }
    return res;
}
int main()
{   //empty = 1
    char message0[] = ""; 
    assert(ReceiveWiFiMesh(message0, strlen(message0), true) == -1);
    //no json format return
    char message1[] = "hoi"; 
    assert(ReceiveWiFiMesh(message1, strlen(message1), true) == -1);

    //no cmd
    char message2[] = "{\"id\" : 1}";
    assert(ReceiveWiFiMesh(message2, strlen(message2), true) == -1);

    //cmd = heartbeat -> without mac & idreturn = - 1 with = 0
    char message3[500];
    snprintf(message3, 500, "{\"%s\":%d,\"%s\":%s}", nameCMD, CMD_HEARTBEAT, nameMAC, "aa:bb:cc");
    assert(ReceiveWiFiMesh(message3, strlen(message3), true) == -1);
    snprintf(message3, 500, "{\"%s\":%d,\"%s\":\"%s\", \"%s\": 1}", nameCMD, CMD_HEARTBEAT, nameMAC, "aa:bb:cc", nameID);
    assert(ReceiveWiFiMesh(message3, strlen(message3), true) == 0);
    //when root -> cmd != server CMD = to server, than is, mac, ip, port & temp 
    char message4[500];
    snprintf(message4, 500, "{\"id\" : 1,\"%s\": %d}", nameCMD, CMD_TO_SERVER);
    assert(ReceiveWiFiMesh(message3, strlen(message3), true)  == 0);
    snprintf(message4, 500, "{\"id\" : 1,\"%s\": %d}", nameCMD, CMD_INIT_SEND);
    assert(ReceiveWiFiMesh(message3, strlen(message3), true)  == 0);
    snprintf(message4, 500, "{\"id\" : 1,\"%s\": %d}", nameCMD, CMD_SET_ERR);
    assert(ReceiveWiFiMesh(message3, strlen(message3), true)  == 0);
    //NOT VALID
    snprintf(message4, 500, "{\"id\" : 1,\"%s\": %d}", nameCMD, CMD_ERROR);
    assert(ReceiveWiFiMesh(message4, strlen(message4), true)  == -1);
    snprintf(message4, 500, "{\"id\" : 1,\"%s\": %d}", nameCMD, CMD_SET_SERVER_NUMBER);
    assert(ReceiveWiFiMesh(message4, strlen(message4), true)  == -3);
    snprintf(message4, 500, "{\"id\" : 1,\"%s\": %d}", nameCMD, CMD_TO_CLIENT);
    assert(ReceiveWiFiMesh(message4, strlen(message4), true)  == -3);
    snprintf(message4, 500, "{\"id\" : 1,\"%s\": %d}", nameCMD, CMD_INIT_INVALID);
    assert(ReceiveWiFiMesh(message4, strlen(message4), true)  == -3);
    snprintf(message4, 500, "{\"id\" : 1,\"%s\": %d}", nameCMD, CMD_INIT_VALID);

    //invalid values
    snprintf(message4, 500, "{\"%s\":%d}", nameCMD, 100);
    assert(ReceiveWiFiMesh(message4, strlen(message4), true) == -3);
    //invalid cmd -> no data change
    curGeneralState = 2;
    snprintf(message4, 500, "{\"%s\":%d,\"%s\":\"%s\",\"%s\": 1, \"%s\": %d}", nameCMD, 100, nameMAC, "aa:bb:cc", nameID,nameGenState, 1);
    assert(ReceiveWiFiMesh(message4, strlen(message4), false) == -1);
    assert(curGeneralState == 2);
    //received temp will not be updated, because root 
    char message5[500];
    curReceivedTemp = 11;
    snprintf(message5, 500, "{\"id\" : 1,\"%s\": %d,\"%s\":%d}", nameCMD, CMD_TO_SERVER, nameTemp, 10);
    ReceiveWiFiMesh(message5, strlen(message5), true);
    assert( curReceivedTemp == 11);
    //received temp will be updated, because leaf
    snprintf(message5, 500, "{\"id\" : 1,\"%s\": %d,\"%s\":%d, \"%s\": \"%s\"}", nameCMD, CMD_TO_CLIENT, nameTemp, 10, nameMAC, "aa:bb:cc");
    ReceiveWiFiMesh(message5, strlen(message5), false);
    fflush(stdout);
    assert( curReceivedTemp == 10);

    //genstate will not be updated because invalid value
    curGeneralState = 0;
    snprintf(message5, 500, "{\"id\" : 1,\"%s\": %d,\"%s\":%d}", nameCMD, CMD_TO_SERVER, nameGenState, -2);
    ReceiveWiFiMesh(message5, strlen(message5), true);
    assert( curGeneralState == 0);
    snprintf(message5, 500, "{\"id\" : 1,\"%s\": %d,\"%s\":%d}", nameCMD, CMD_TO_SERVER, nameGenState, 3);
    ReceiveWiFiMesh(message5, strlen(message5), true);
    assert( curGeneralState == 0);
    //genstate will be updated because valid value
    curGeneralState = 0;
    snprintf(message5, 500, "{\"id\" : 1,\"%s\": %d,\"%s\":%d}", nameCMD, CMD_TO_SERVER, nameGenState, -1);
    ReceiveWiFiMesh(message5, strlen(message5), true);
    assert( curGeneralState == -1);
    //voltage state will not be updated because valid value
    curVoltageState = 0;
    snprintf(message5, 500, "{\"id\" : 1,\"%s\": %d,\"%s\":%d}", nameCMD, CMD_TO_SERVER, nameVoltageState, -1);
    ReceiveWiFiMesh(message5, strlen(message5), true);
    assert( curVoltageState == 0);
    snprintf(message5, 500, "{\"id\" : 1,\"%s\": %d,\"%s\":%d}", nameCMD, CMD_TO_SERVER, nameVoltageState, 2);
    ReceiveWiFiMesh(message5, strlen(message5), true);
    assert( curVoltageState == 0);
    //voltage state will  be updated because valid value
    curVoltageState = 0;
    snprintf(message5, 500, "{\"id\" : 1,\"%s\": %d,\"%s\":%d}", nameCMD, CMD_TO_SERVER, nameVoltageState, 1);
    ReceiveWiFiMesh(message5, strlen(message5), true);
    assert( curVoltageState == 1);

    //leaf:
    //invalid cmd return -1
    char message6[500];
    snprintf(message6, 500, "{\"%s\":%d}", nameCMD, CMD_TO_SERVER);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == -1);
    snprintf(message6, 500, "{\"%s\":%d}", nameCMD, CMD_ERROR);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == -1);
    snprintf(message6, 500, "{\"%s\":%d}", nameCMD, CMD_INIT_SEND);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == -1);
    snprintf(message6, 500, "{\"%s\":%d}", nameCMD, CMD_SET_ERR);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == -1);
    snprintf(message6, 500, "{\"%s\":%d}", nameCMD, CMD_SET_SERVER_NUMBER);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == -1);
    //invalid cmd
    snprintf(message6, 500, "{\"%s\":%d}", nameCMD, 100);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == -1);
    //invalid cmd -> no data change
    curGeneralState = 2;
    snprintf(message6, 500, "{\"%s\":%d,\"%s\":\"%s\",\"%s\": 1, \"%s\": %d}", nameCMD, 100, nameMAC, "aa:bb:cc", nameID,nameGenState, 1);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == -1);
    assert(curGeneralState == 2);
    //valid
    snprintf(message6, 500, "{\"%s\":%d, \"%s\":\"%s\", \"%s\":%d}", nameCMD, CMD_INIT_INVALID, nameMAC, "aa:bb:cc", nameID, 1);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == 0);
    snprintf(message6, 500, "{\"%s\":%d}", nameCMD, CMD_INIT_VALID);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == 0);
    snprintf(message6, 500, "{\"%s\":%d}", nameCMD, CMD_TO_CLIENT);
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == 0);
    
    //mac != own mac
    snprintf(message6, 500, "{\"%s\":%d,\"%s\":\"%s\"}",nameCMD, CMD_TO_CLIENT, nameMAC, "aa:bb:ca");
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == -1);
    //mac == own mac
    snprintf(message6, 500, "{\"%s\":%d,\"%s\":\"%s\"}", nameCMD, CMD_TO_CLIENT, nameMAC, "aa:bb:cc");
    assert(ReceiveWiFiMesh(message6, strlen(message6), false) == 0);
    
    snprintf(message6, 500, "{\"%s\":%d,\"%s\":\"%s\",\"%s\": 1, \"%s\": %d}", nameCMD, CMD_TO_CLIENT, nameMAC, "aa:bb:cc", nameID,nameGenState, 1);
    ReceiveWiFiMesh(message6, strlen(message6), false);
    assert(curGeneralState == 1);
    snprintf(message6, 500, "{\"%s\":%d,\"%s\":\"%s\", \"%s\": %d}", nameCMD, CMD_TO_CLIENT, nameMAC, "aa:bb:cc", nameVoltageState, 1);
    ReceiveWiFiMesh(message6, strlen(message6), false);
    assert(curVoltageState == 1);
    snprintf(message6, 500, "{\"%s\":%d,\"%s\":\"%s\", \"%s\": %d}", nameCMD, CMD_TO_CLIENT, nameMAC, "aa:bb:cc", nameTemp, 1);
    ReceiveWiFiMesh(message6, strlen(message6), false);
    assert(curReceivedTemp == 1);
    printf("Completed\n");


}
/*
int main()
{
    //no json format return
    mesh_data ret;
    char message[] = "hoi"; 
    assert(receive(&ret,message) < 0);
    
    //no cmd
    mesh_data ret2;
    char message2[] = "{\"id\" : 1}";
    assert(receive(&ret2,message2) < 0);
    
    //if CMD = to server, than is, mac, ip, port & temp 
    mesh_data ret3;
    char message3[500];
    snprintf(message3, 500, "{\"%s\": 1, \"%s\":%d,\"%s\":%s}", nameID, nameCMD, CMD_TO_SERVER, nameMAC, "aa:bb:cc");
    assert(receive(&ret3,message3) < 0);
    
    //if CMD = heartbeat ->ID should be present
    mesh_data ret4;
    char message4[500];
    snprintf(message4, 500, "{\"%s\":%d,\"%s\":%s}", nameCMD, CMD_HEARTBEAT, nameMAC, "aa:bb:cc");
    assert(receive(&ret4,message4) < 0);  

    //mac should be represent
    mesh_data ret5;
    char message5[500];
    snprintf(message5, 500, "{\"%s\":%d,\"%s\":1}", nameCMD, CMD_HEARTBEAT, nameID);
    assert(receive(&ret5, message5) < 0);

    //mac & ip are represent, so return -3, because there is no return and so no mac in r struct
    mesh_data ret6;
    char message6[500];
    snprintf(message6, 500, "{\"%s\":%d,\"%s\":1, \"%s\":\"%s\"}", nameCMD, CMD_HEARTBEAT, nameID, nameMAC, "aa:bb:cc");
    assert(receive(&ret6, message6) == -3);

    //mac & ip are represent, so return -1 bij CMD = TO_SERVER, because toServer is not allowed for a lvi
    mesh_data ret7;
    char message7[500];
    snprintf(message7, 500, "{\"%s\":%d,\"%s\":1, \"%s\":\"%s\"}", nameCMD, CMD_TO_SERVER, nameID, nameMAC, "aa:bb:cc");
    assert(receive(&ret7, message7) == -1);

    //own mac not equal to sending mac = returning -3;
    mesh_data ret8;
    char message8[500];
    snprintf(message8, 500, "{\"%s\":%d,\"%s\":1, \"%s\":\"%s\"}", nameCMD, CMD_TO_SERVER, nameID, nameMAC, "aa:bb:ca");
    assert(receive(&ret8, message8) == -3);
    printf("Completed\n");
}
*/