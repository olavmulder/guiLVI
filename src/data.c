#include "../include/data.h"

//cjson handling
const char *nameVoltageState = "voltageState";
const char *nameCloseState = "closeState";
const char *nameTemp = "temp";
const char *nameCounter = "counter";
const char *nameClientID = "clientID";
const char *nameID = "id";
const char *nameType = "type";
const char *nameActive = "active";
const char *nameArray = "array";
const char *nameMAC = "mac";
const char *nameIP = "ip";
const char *namePort = "port";
const char *nameCMD = "cmd";
const char *nameACK = "ack" ;
const char* nameIsAlive = "isAlive";

static init_t initList[AMOUNT_NODES];
DataList *dataListPtr = &list[0];

/**
 * @brief checks if id already exits
 * changed ret->cmd to init_valid/init_invalid
 * 
 * @param ret 
 */
int _CheckIDExist(mesh_data *ret)
{
   //check if id is already taken
   if(ret->id < 0 || ret->id > sizeof(initList)/sizeof(init_t))
      return -1;
   if(initList[ret->id].init == true)
   {
      //if so->check if incoming mac & ip is same
      if(strcmp(initList[ret->id].mac, ret->mac) == 0 &&
         strcmp(initList[ret->id].ip, ret->ip) == 0)
      {
         //if so it is a valid init
         //ret->cmd = CMD_INIT_VALID;
         return 1;
      }
      else
      {
         return 0;
         //ret->cmd = CMD_INIT_INVALID;
      }
   }else{//if id is not init = it is a new one then set mac + ip
            
      strcpy(initList[ret->id].mac, ret->mac);
      strcpy(initList[ret->id].ip, ret->ip);
      initList[ret->id].init = true;
      //ret->cmd = CMD_INIT_VALID;
      return 1;
   }
   

}

/**
 * @brief return 1 when sync init
 * 
 * @param ret mesh_data structure which will be used to generate returning msg
 * @param buffer 
 * @param len 
 * @return int
 */
int HandleIncomingData(mesh_data *ret, char* buffer, size_t len)
{
   int res = -1;
   cJSON * jsonCMD = NULL;
   cJSON * jsonID = NULL;
   cJSON *objPtr = cJSON_ParseWithLength(buffer, len);
   if(objPtr == NULL)
   {
      printf("%s objptr == NULL\n", __func__);
      return _cJSON_ErrorHandling(objPtr);
   }
   //get command type
   jsonCMD = cJSON_GetObjectItemCaseSensitive(objPtr, nameCMD);
   if(cJSON_IsNumber(jsonCMD))
   {
      int cmd = jsonCMD->valueint;
      //run function demending on cmd 
      //printf("%s, cmd = %d\n", __func__, cmd);
      switch(cmd)
      {
         //lvi commands
         case CMD_INIT_SEND:
            res = HandleClientData(ret, objPtr);
            if(_CheckIDExist(ret) == 1)ret->cmd = CMD_INIT_VALID;
            else ret->cmd = CMD_INIT_INVALID;
            //printf("HandleClientData form init send -> %d\n", res);
            break;
         case CMD_TO_SERVER:
            res = HandleClientData(ret, objPtr);
            if(res == 0){
               ret->cmd = CMD_TO_CLIENT;
               ret->closeState = list[ret->id].closeState;
               ret->voltageState = list[ret->id].voltageState;
            }
            printf("HandleClientData -> %d\n", res);

            break;
         case CMD_HEARTBEAT:
            ret->cmd = CMD_HEARTBEAT;
            res = HandleHeartBeat(ret, objPtr);
            printf("%s, cmd%d\n", __func__, ret->cmd);
            //on lvi there is check on ip so set it.
            break;
         case CMD_SEND_ERR:
            jsonID = cJSON_GetObjectItemCaseSensitive(objPtr, nameID);
            if(cJSON_IsNumber(jsonID))
            {
               int id = jsonID->valueint;
               *ret =  HandleSendErr(id);
            }
            break;
         //sync commands
         case CMD_SYNC_INIT://when sync init send copy
            //printf("%s; received CMD_SYNC_INIT\n", __func__);
            return 1;
            break;
         case CMD_SYNC:
         case ASK_COPY_ID:
         case RECEIVE_COPY_ID:
            res = ReadSyncCallBack(buffer, strlen(buffer));
            ret->cmd = CMD_SYNC;
            //printf("ReadSyncCallback -> %d\n", res);
            break;
         case CMD_SEND_BROADCAST:
            res = 0;
            break;
         default:
            break;
      }      
   }
   cJSON_Delete(objPtr);
   if(ret->cmd == CMD_ERROR)return -1;
   return res;
}

int _MakeMsgLvi(mesh_data *ret, char*msgStr, size_t len)
{
      //printf("%s;%d cmd\n", __func__, ret->cmd);
      //ip:%s, mac: %s, id: %d, port %d, id %d\n", ret->cmd , ret->ip, ret->mac, 
      //   ret->port,  ret->id
      cJSON *obj = cJSON_CreateObject();
      if (obj == NULL)
      {
         goto end;
      }
      if (cJSON_AddStringToObject(obj, nameIP, ret->ip) == NULL)
      {
         goto end;
      }
      if (cJSON_AddNumberToObject(obj, namePort, ret->port) == NULL)
      {
         goto end;
      }
      if (cJSON_AddStringToObject(obj, nameMAC, ret->mac) == NULL)
      {
         goto end;
      }
      if (cJSON_AddNumberToObject(obj, nameCMD, ret->cmd) == NULL)
      {
         goto end;
      }
      if (cJSON_AddNumberToObject(obj, nameID, ret->id) == NULL)
      {
         goto end;
      }
        
      if(ret->cmd == CMD_TO_CLIENT)
      {
         if (cJSON_AddNumberToObject(obj, nameCloseState, ret->closeState) == NULL)
         {
            goto end;
         }

         if (cJSON_AddNumberToObject(obj, nameVoltageState, ret->voltageState) == NULL)
         {
            goto end;
         }
         

         if (cJSON_AddNumberToObject(obj, nameTemp, ret->temp) == NULL)
         {
            goto end;
         }
      }
      //check if size is not to big, if so copy to string
      char *tempStr  = cJSON_Print(obj);
      if (msgStr == NULL)
      {
         fprintf(stderr,"Failed to print obj\n");
      }
      else if(strlen(tempStr) < len)
      {
         strcpy(msgStr, tempStr);
         free(tempStr);
      }
      cJSON_Delete(obj);
      return 0;
      end:
         cJSON_Delete(obj);
         return -1;

}


mesh_data HandleSendErr(int id)
{
   mesh_data ret;
   //if current general state == Err,  make changelog to send
   if(id < (int)(sizeof(list)/sizeof(DataList))){
            if(MakeChangeLog(id, 0,-1,0, false, false, 
                     true,false,false) != 0)
            {
               fprintf(stderr, "Error in MakechangeLog\n");
            }
            //printf("send err on list[%d].curStatte: %d\n", id, list[id].closeState);
   }
   ret.cmd = CMD_TO_CLIENT;
   return ret;
}
int HandleClientData(mesh_data *ret, cJSON* objPtr)
{
   cJSON* jsonMAC = cJSON_GetObjectItemCaseSensitive(objPtr, nameMAC);
   cJSON* jsonID = cJSON_GetObjectItemCaseSensitive(objPtr, nameID);
   cJSON* jsonIP = cJSON_GetObjectItemCaseSensitive(objPtr, nameIP);
   cJSON* jsonPort = cJSON_GetObjectItemCaseSensitive(objPtr, namePort);
   cJSON* jsonTemp = cJSON_GetObjectItemCaseSensitive(objPtr, nameTemp);
          
   if(!
      (cJSON_IsNumber(jsonID) &&
      cJSON_IsString(jsonMAC) &&
      cJSON_IsString(jsonIP) &&
      cJSON_IsNumber(jsonPort) &&
      cJSON_IsNumber(jsonTemp))
   ){
      printf("%s:not everything is in json\n", __func__);
      fflush(stdout);
      ret->cmd = CMD_ERROR;
      return -1;
   }  

   strcpy(ret->mac,jsonMAC->valuestring);
   strcpy(ret->ip,jsonIP->valuestring);
   ret->port =(uint16_t)jsonPort->valueint;
   ret->id = (unsigned int)jsonID->valueint;
   ret->temp = jsonTemp->valuedouble;
   //if temperature data has chagned set make changelog to send
   //changed data
   if(ret->id < (int)(sizeof(list)/sizeof(DataList))){
      if(list[ret->id].temp != jsonTemp->valuedouble)
      {
         if(MakeChangeLog(ret->id, 0,0,jsonTemp->valuedouble, false, 
                  false, false,true,false ) <= 0)
         {
            fprintf(stderr, "Error in MakechangeLog\n");
            fflush(stderr);
            //return -1;
         }
      }
   }      
   return 0;
}
int HandleHeartBeat(mesh_data *ret, cJSON *objPtr)
{
   static int init = 0;
   if(init == 0)
   {
      monitoring_head = malloc(sizeof(strip_t));
      monitoring_head->childArr =(Node**)malloc(sizeof(Node*));
      //monitoring_head->childArr[0] =(Node*)malloc(sizeof(Node));
      monitoring_head->lenChildArr = 0;
      init = 1;
   }
   //handle heartbeat...
   ret->cmd = CMD_HEARTBEAT;
   cJSON *jsonID = cJSON_GetObjectItemCaseSensitive(objPtr, nameID);
   
   if(! (cJSON_IsNumber(jsonID)) )
   {
      printf("%s:id is null", __func__);
      return -1;
   }else{
      cJSON *node = NULL;
      cJSON *data = NULL;

      //setup return values
      ret->id  = jsonID->valueint;
      snprintf(ret->mac,20, "%s", "00");
      snprintf(ret->ip,20, "%s", ownAddr_);
      if(strcmp(ownAddr_, "192.168.2.103") == 0)
         ret->id = 255;//test a server id
      else 
         ret->id = 254;
      ret->port = 0;

      //printf("%s: call heartbeat handler: id:%d\n", __func__, jsonID->valueint);
      fflush(stdout);
      strip_t *child = (strip_t*)malloc(sizeof(strip_t));
      child->childArr = (Node**)malloc(sizeof(Node*));
      child->childArr[0] = (Node*)malloc(sizeof(Node));
      child->lenChildArr = 1;
      
      //data(id, mac, ip, port) of sender
      child->childArr[0]->id = jsonID->valueint;
      cJSON *jsonIP = cJSON_GetObjectItemCaseSensitive(objPtr, nameIP);
      cJSON *jsonMAC = cJSON_GetObjectItemCaseSensitive(objPtr, nameMAC);

      if(cJSON_IsString(jsonIP))
         snprintf(child->childArr[0]->ip_wifi, 20, "%s", jsonIP->valuestring);
      else
         snprintf(child->childArr[0]->ip_wifi, 20, "not there");
          
      //discard
      child->childArr[0]->port = 0;
      if(cJSON_IsString(jsonMAC))
         snprintf(child->childArr[0]->mac_wifi, 20, "%s", jsonMAC->valuestring);
      else
         snprintf(child->childArr[0]->mac_wifi, 20, "not there");
      
      child->childArr[0]->isAlive = true;

   
      data = cJSON_GetObjectItemCaseSensitive(objPtr, "array");
      if(data != NULL)
      {
         cJSON_ArrayForEach(node, data)
         {
            //all the data of the sender's childs
            Node *temp = malloc(sizeof(Node));
            cJSON *mac_in_Arr = cJSON_GetObjectItemCaseSensitive(node, nameMAC);
            cJSON *ip_in_Arr = cJSON_GetObjectItemCaseSensitive(node, nameIP);
            cJSON *id_in_Arr = cJSON_GetObjectItemCaseSensitive(node, nameID);
            cJSON *port_in_Arr = cJSON_GetObjectItemCaseSensitive(node, namePort);
            cJSON *isAlive_in_Arr = cJSON_GetObjectItemCaseSensitive(node, nameIsAlive);
            
            if(cJSON_IsNumber(id_in_Arr))
                  temp->id = id_in_Arr->valueint;
            if(cJSON_IsString(ip_in_Arr))
                  strcpy(temp->ip_wifi, ip_in_Arr->valuestring);
            if(cJSON_IsString(mac_in_Arr))
                  strcpy(temp->mac_wifi, mac_in_Arr->valuestring);
            if(cJSON_IsNumber(port_in_Arr)) 
                  temp->port = port_in_Arr->valueint; 
            if(cJSON_IsNumber(isAlive_in_Arr))   
                  temp->isAlive = isAlive_in_Arr->valueint;

            child->childArr = realloc(child->childArr, sizeof(Node)* child->lenChildArr+1);
            child->childArr[child->lenChildArr] = temp;
            child->lenChildArr++;
         }
      }
      //printf("%s: id:%d, child->len %d\n", __func__, child->childArr[0]->id, child->lenChildArr);
      fflush(stdout);
      HeartbeatHandler(jsonID->valueint, child);
      //free malloced strip_t & childArr from HandleCMD function
      //ESP_LOGI(TAG_DATA, "%s, lenchild %d", __func__, child->lenChildArr);
      for(uint8_t i = 0; i < child->lenChildArr;i++)
         free(child->childArr[i]);
      free(child->childArr);
      free(child);
      //ESP_LOGI(TAG_DATA, "%s free done", __func__); 
   }
   return 0;
}