#include "../include/utils.h"
#define SERVER_ID1 255
#define SERVER_ID2 254

int othersID()
{
   if(strlen(ownAddr_) == 0)return -1;

   if(strcmp(ownAddr_, SERVER_IP[0]) == 0)return SERVER_ID2;
   if(strcmp(ownAddr_, SERVER_IP[1]) == 0)return SERVER_ID1;
   return -1;
}

int myID()
{
   if(strlen(ownAddr_) == 0)return -1;
   
   if(strcmp(ownAddr_, SERVER_IP[0]) == 0)return SERVER_ID1;
   if(strcmp(ownAddr_, SERVER_IP[1]) == 0)return SERVER_ID2;
   return -1;
}
bool is_ID_Server(int id, int id2)
{
   
   if(id == SERVER_ID1 || id2 == SERVER_ID1)return true;
   if(id == SERVER_ID2 || id2 == SERVER_ID2)return true;
   return false;
}
bool is_CMD_a_Return_Msg(CMD cmd)
{
   switch(cmd)
   {
      case  CMD_INIT_INVALID: 
      case  CMD_INIT_VALID:   
      case  CMD_TO_CLIENT:
      case  CMD_HEARTBEAT:
         return true;
      default:
         return false;
   }
   return false;
}