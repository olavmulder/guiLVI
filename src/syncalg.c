#include "../include/syncAlg.h"

#define PORT 8080
uint8_t clientId = 0;
DataList list[LIST_SIZE] = {0};
unsigned int highestID = 0;
uint64_t syncCounter = 0;
volatile bool updateFlag  = false;

int _cJSON_ErrorHandling(cJSON *obj)

{
   const char *error_ptr = cJSON_GetErrorPtr();
   if (error_ptr != NULL)
   {
      fprintf(stderr, "Error before: %s\n", error_ptr);
   }
   cJSON_Delete(obj);
   return -1;
}

int _HandleSync(cJSON* objPtr, int id)
{
    DataList newData;
    cJSON *jsonVoltageState = NULL;
    cJSON *jsonCloseState = NULL;
    cJSON *jsonTemp = NULL;
    cJSON *jsonActive = NULL;

    jsonVoltageState = cJSON_GetObjectItemCaseSensitive(objPtr, nameVoltageState);
    jsonCloseState = cJSON_GetObjectItemCaseSensitive(objPtr, nameCloseState);
    jsonTemp = cJSON_GetObjectItemCaseSensitive(objPtr, nameTemp);
    jsonActive = cJSON_GetObjectItemCaseSensitive(objPtr, nameActive);

    bool uVolt = false, uTemp = false, uClose = false, uActive = false;

    if(cJSON_IsNumber(jsonVoltageState))
    { 
        newData.voltageState = (int8_t)jsonVoltageState->valueint;
        uVolt = true;
    }
    if(cJSON_IsNumber(jsonTemp)) 
    {
        newData.temp = jsonTemp->valuedouble;
        uTemp = true;
    }
    if(cJSON_IsNumber(jsonCloseState) ) 
    {
        newData.closeState = (int8_t)jsonCloseState->valueint;
        uClose = true;
    }
    if(cJSON_IsBool(jsonActive)) 
    {
        newData.active = jsonActive->valueint;
        uActive = true;
    }
    newData.id = id;
    fflush(stdout);
    //change received data in array list
    return _ChangeData(&newData, uVolt, uClose, uTemp, uActive);
}
int _ReceiveChangeLog(const char* const msg)
{   

    cJSON *objPtr = cJSON_Parse(msg);
    cJSON *jsonId = NULL;
    cJSON *jsonCounter = NULL;
    
    //check on valid json format
    if(objPtr == NULL)
    {
        _cJSON_ErrorHandling(objPtr);
        return -4;
    }

    jsonId = cJSON_GetObjectItemCaseSensitive(objPtr, nameID);
    jsonCounter = cJSON_GetObjectItemCaseSensitive(objPtr, nameCounter);
    
    //if id & counter is in json msg
    if (cJSON_IsNumber(jsonId) &&
        cJSON_IsNumber(jsonCounter) )
    {
        //check if received id ask for a copy of the data
        if(jsonId->valueint == ASK_COPY_ID){
            char* r =  SendCopyData();
            return SendSync(r, strlen(r));
        }
        //check if receive copy is send
        else if(jsonId->valueint == RECEIVE_COPY_ID)
        {
            //wait till gui is init, otherwise copy data is replaced by init data...
            while(!canvasInitDone)continue;
            return ReceiveCopyData(objPtr, jsonCounter);
        }
        //only do something when syncounter is smaller than incoming
        //if synccounter is valid handle sync
        else if(syncCounter < (uint64_t)jsonCounter->valueint)
        {
            syncCounter = (uint64_t)jsonCounter->valueint;
            return _HandleSync(objPtr, jsonId->valueint);
            
        }//end valid synccounter
        
        //return -2 when sync counter is not smaller enought to do something
        //return -2 when client id is same is own clientid
        return -2;
    }
    //return -3 when json data doenst contain id and counter;
    return -3;
}
int ReadSyncCallBack(const char* msg, size_t len)
{
        //printf("%s; synccounter %ld\n", __func__, syncCounter);
        int res = _ReceiveChangeLog(msg);
        switch(res){
            case 0:
                return 0;
                break;
            case -1:
                fprintf(stderr, "couldn't not change data\n");
                break;
            case -2:
                fprintf(stderr, "data is to old to update\n");
                break;
            case -3:
                fprintf(stderr, "invalid message, no id in json\n");
                break;
            case -4:
                fprintf(stderr, "json error\n");
                break;
            default:
                fprintf(stderr, "no known feedback from receive changelog\n");
                break;
        }
        return res;
}

/**
 * @brief Make cjson message to send, change self the data and actuall 
 *  send the json message to peer, set 'u....'  varibale to true, to actuale change 
 * variable
 * 
 * @param id 
 * @param voltageState 
 * @param closeState 
 * @param temp 
 * @param active 
 * @param uVolt 
 * @param uClose 
 * @param uTemp 
 * @param uActive 
 * @return int -1 on error, 0 on OK
 */
int MakeChangeLog(unsigned int id, int8_t voltageState, int8_t closeState, double temp,
                            bool active, bool uVolt, bool uClose, bool uTemp, bool uActive)
{
    
    DataList data;
    if(uVolt || uClose || uTemp || uActive)
    {
        syncCounter++;
        cJSON *changeMsg = cJSON_CreateObject();
        
        if(id > highestID)highestID = id;
        
        if(cJSON_AddNumberToObject(changeMsg, nameID, id) == NULL)
        {
            cJSON_Delete(changeMsg);
            return -1;
        }
        if(cJSON_AddNumberToObject(changeMsg, nameCMD, CMD_SYNC) == NULL)
        {
            cJSON_Delete(changeMsg);
            return -1;
        }
        if(cJSON_AddNumberToObject(changeMsg, nameCounter, syncCounter) == NULL)
        {
            
            cJSON_Delete(changeMsg);
            return -1;
        }

        data.id = id;
        if(uVolt)
        {
            data.voltageState = voltageState;
            if(cJSON_AddNumberToObject(changeMsg, nameVoltageState, voltageState) == NULL)
            {
                
                cJSON_Delete(changeMsg);
                return -1;
            }
        }        
        if(uClose)
        {
            data.closeState = closeState;

            if(cJSON_AddNumberToObject(changeMsg, nameCloseState, closeState) == NULL)
            {
                cJSON_Delete(changeMsg);
                return -1;
            }
        }
        if(uTemp)
        {
            data.temp = temp;
            if(cJSON_AddNumberToObject(changeMsg, nameTemp, temp) == NULL)
            {
                cJSON_Delete(changeMsg);
                return -1;
            }
        }
        if(uActive)
        {
            data.active = active;
            if(cJSON_AddNumberToObject(changeMsg, nameActive, active) == NULL)
            {
                cJSON_Delete(changeMsg);
                return -1;
            }
        }

        if(_ChangeData(&data, uVolt, uClose, uTemp, uActive) < 0)
        {
            printf("%s: change data -1\n", __func__);
            fflush(stdout);
        }

        char* str = cJSON_Print(changeMsg);
        if(str == NULL){
            fprintf(stderr, "failed to print changeMsg\n");
            return -1;
        }
        int res = SendSync(str, strlen(str));
        free(str);
        return res;
    }
    else{
        return -1;
    }
}

//give id of change and set value + bool value to actually change data
/**
 * @brief Change data in array of position id depending if u'Variable' is set
 * also sets variable hasChanged to true
 * @param DataList* struct filed with data 
 * @param uVolt 
 * @param uClose 
 * @param uTemp 
 * @param uActive 
 * @return int -1 on failure 0 on OK
 */
int _ChangeData(DataList *data, bool uVolt, bool uClose,
                        bool uTemp, bool uActive)
{
    unsigned int id = data->id;
    if(monitoring_head != NULL)
    {
        if(monitoring_head->lenChildArr > 0){//is inited
            int index = FindID(id);
            if(index >= 0)//has found
            {
                //if incomfing data is closestate Err 
                if(uClose && data->closeState == -1)
                {
                    //if this device is alive do not change close state
                    if(monitoring_head->childArr[index]->isAlive == true)
                    {
                        uClose = false;
                    }
                }
            } 

        }
    }
    if(id > LIST_SIZE ){
        fprintf(stderr, "id: %d;  isn't valid\n", data->id);
        fflush(stdout);
        return -1;
    }else{
        list[id].id = id;
    }
    if(uVolt || uClose || uTemp || uActive){
        if(uVolt)
        {
            if(data->voltageState == 0 || data->voltageState == 1)
                list[id].voltageState = data->voltageState;
        }
        if(uClose)
        {
            if(data->closeState >= -1 && data->closeState <= 2)
                list[id].closeState = data->closeState;
        }
        if(uTemp)
        {
            list[id].temp = data->temp;
        }
        if(uActive){
            list[id].active = data->active;
        }
    }
    else{
        fprintf(stderr, "cant't change data because all variables are false\n");
        return -1;
    }
    return 0;
}

#ifdef DEBUG
void PrintList(FILE* pFile)
{

    fprintf(pFile,"-------------------");
    for(uint32_t i = 0; i < 5;i++)
    {
    
        fprintf(pFile, "id: %d, closeState: %d, temp: %.2f, voltageState: %d\n", 
            list[i].id, list[i].closeState, list[i].temp, list[i].voltageState);
    }
    fprintf(pFile,"\n\n");
    fprintf(pFile,"-------------------");

}

/**
 * @brief send copy of all data 
 * 
 * @return int -1 on failure 0 on OK
 */
char* SendCopyData()
{
    
    cJSON *array = NULL;
    cJSON *dList = NULL;
    
    cJSON *jsonVoltageState = NULL;
    cJSON *jsonCloseState = NULL;
    cJSON *jsonTemp = NULL;
    cJSON *jsonActive = NULL;
    
    char* str;

    cJSON *data = cJSON_CreateObject();
    if(data == NULL){
        //error
        return NULL;
    }
    if(cJSON_AddNumberToObject(data, nameID, RECEIVE_COPY_ID)  == NULL)
    {
        cJSON_Delete(data);
        return NULL;
    }
    if(cJSON_AddNumberToObject(data, nameCounter, ++syncCounter)  == NULL)
    {
        cJSON_Delete(data);
        return NULL;
    }

    if(cJSON_AddNumberToObject(data, nameCMD, CMD_SYNC)  == NULL)
    {
        cJSON_Delete(data);
        return NULL;
    }
    array = cJSON_CreateArray();
    if(array == NULL){
        return NULL;
    }
    cJSON_AddItemToObject(data, nameArray, array);
    for(uint16_t i = 0; i <= 7/*highestID*/;i++)
    {
        dList = cJSON_CreateObject();
        if(dList == NULL){
            return NULL;
        }
        cJSON_AddItemToArray(array, dList);
        //add voltage
        jsonVoltageState = cJSON_CreateNumber(list[i].voltageState);
        if(jsonVoltageState == NULL)
        {
            return NULL;
        }
        cJSON_AddItemToObject(dList, nameVoltageState, jsonVoltageState);
        //add close state
        jsonCloseState = cJSON_CreateNumber(list[i].closeState);
        if(jsonCloseState == NULL)
        {
            return NULL;
        }
        cJSON_AddItemToObject(dList, nameCloseState , jsonCloseState);
        //add temp
        jsonTemp = cJSON_CreateNumber(list[i].temp);
        if(jsonTemp == NULL)
        {
            return NULL;
        }
        cJSON_AddItemToObject(dList, nameTemp, jsonTemp);
        //add active
        jsonActive = cJSON_CreateNumber(list[i].active);
        if(jsonActive == NULL)
        {
            return NULL;
        }
        cJSON_AddItemToObject(dList, nameActive, jsonActive);     
    }
    
    str = cJSON_Print(data);
    if(str == NULL){
        fprintf(stderr, "failed to print changeMsg\n");
        return NULL;
    }
    return str;
}

int ReceiveCopyData(cJSON *objPtr, cJSON*jsonCounter)
{
    printf("receive copy data\n");
    fflush(stdout);
    cJSON* tem;
    unsigned int i = 0;
    bool go = false, temp =false;
    DataList newData;
    //check if all nodes are init in the gtk dropdown callback
    //otherwise the send copy data will be overwritten
    do{
        for(uint8_t i=0;i<AMOUNT_NODES;i++){
            if(firstTimeCallBack[i] == true)temp = true;
            else temp = false;
        }
        if(temp == true)
            go = true;
    }while(!go);
    fflush(stdout);
    cJSON*jsonArray = cJSON_GetObjectItemCaseSensitive(objPtr, nameArray);
    //update syncounter variable
    syncCounter = (uint64_t)jsonCounter->valueint;
    printf("synccounter: %ld\n", syncCounter);
    fflush(stdout);
    if(cJSON_IsArray(jsonArray)){
        cJSON_ArrayForEach(tem, jsonArray){

            cJSON* jsonVoltageState = cJSON_GetObjectItemCaseSensitive(tem, nameVoltageState);
            cJSON* jsonCloseState = cJSON_GetObjectItemCaseSensitive(tem, nameCloseState);
            cJSON* jsonTemp = cJSON_GetObjectItemCaseSensitive(tem, nameTemp);
            cJSON* jsonActive = cJSON_GetObjectItemCaseSensitive(tem, nameActive);

            bool uVolt = false, uTemp = false, uClose = false, uActive = false;
            if(cJSON_IsNumber(jsonVoltageState) && !cJSON_IsNull(jsonVoltageState))
            { 
                newData.voltageState = (int8_t)jsonVoltageState->valueint;
                uVolt = true;
            }
            if(cJSON_IsNumber(jsonTemp) && !cJSON_IsNull(jsonTemp)) 
            {
                newData.temp = jsonTemp->valuedouble;
                uTemp = true;
            }
            if(cJSON_IsNumber(jsonCloseState) && !cJSON_IsNull(jsonCloseState)) 
            {
                newData.closeState = (int8_t)jsonCloseState->valueint;
                uClose = true;
            }
            if(cJSON_IsBool(jsonActive) && !cJSON_IsNull(jsonActive)) 
            {
                newData.active = (bool)jsonActive->valueint;
                uActive = true;
            }
            newData.id = i;
            i++;
            _ChangeData(&newData, uVolt, uClose, uTemp, uActive);
        }
        printf("copy received\n");

    }else{
        printf("no array");
        return -4;
    }
    fflush(stdout);
    return 0;
}
/**
 * @brief request a copy of the data of the other device
 * 
 * @return int -1 on failure, 0 on OK
 */
int AskCopyData()
{
    printf("ask copy data\n");
    cJSON *changeMsg = cJSON_CreateObject();
    if(cJSON_AddNumberToObject(changeMsg, nameID, ASK_COPY_ID)  == NULL)
    {
        cJSON_Delete(changeMsg);
        return -1;
    }
    if(cJSON_AddNumberToObject(changeMsg, nameCounter, ++syncCounter)  == NULL)
    {
        cJSON_Delete(changeMsg);
        return -1;
    }
    if(cJSON_AddNumberToObject(changeMsg, nameCMD, CMD_SYNC)  == NULL)
    {
        cJSON_Delete(changeMsg);
        return -1;
    }
    char* str = cJSON_Print(changeMsg);
    if(str == NULL){
        fprintf(stderr, "failed to print changeMsg\n");
        return -1;
    }
    SendSync(str, strlen(str));
    return 0;

}
//is subject of 
int InitMsg()
{
    cJSON *obj = cJSON_CreateObject();
    if(obj == NULL){
        return -1;
    }
    if(cJSON_AddNumberToObject(obj, nameCMD, CMD_SYNC_INIT)  == NULL)
    {
        cJSON_Delete(obj);
        return -1;
    }
    char* str = cJSON_Print(obj);
    if(str == NULL){
        fprintf(stderr, "failed to print changeMsg\n");
        return -1;
    }
    SendSync(str, strlen(str));
    free(str);
    return 0;
}
#endif