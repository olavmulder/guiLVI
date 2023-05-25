#include "../include/utils.h"

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