#include "../include/utils.h"
#define SERVER_ID1 255
#define SERVER_ID2 254
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