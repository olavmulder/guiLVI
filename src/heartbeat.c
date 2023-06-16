#include "../include/heartbeat.h"

#define TIMER_INTERVAL_SEC 10
#define TIMER_INTERVAL_NSEC 0


strip_t *monitoring_head;
static isInit = false;
bool isHeartbeatInit()
{ 
   return isInit;
}
bool HeartbeatInit()
{
   if (isInit == false)
   {
      monitoring_head = malloc(sizeof(strip_t));
      monitoring_head->childArr = (Node **)malloc(sizeof(Node *));
      // monitoring_head->childArr[0] =(Node*)malloc(sizeof(Node));
      monitoring_head->lenChildArr = 0;
      isInit = true;
      return true;
   }
   return false;
}
void TimerHandler(int sig, siginfo_t *si, void *uc)
{
   Node* data = (Node*)(si->si_value.sival_ptr);
   printf("timer expired of id:%d\n",data->id);
   //increment time out before closing the socket
   data->timeouts ++;
   if(is_ID_Server(data->id, -1) && data->timeouts >= 3)
   {
      CloseSyncSocket();
      fflush(stdout);
      data->timeouts = 0;
   }
   RestartTimer(data->timerid); 
   SetAlive(data, false);
   //remove
   //RemoveFromStrip(monitoring_head, data->id);
}

void InitTimer(Node* node)
{  
   struct sigevent sev;
   struct itimerspec its;
   struct sigaction sa;
   
   // Set up the signal handler
   sa.sa_flags = SA_SIGINFO;
   sa.sa_handler = TimerHandler;
   sigemptyset(&sa.sa_mask);
   sigaction(SIGALRM, &sa, NULL);
   
   // Create the timer
   sev.sigev_notify = SIGEV_SIGNAL;
   sev.sigev_signo = SIGALRM;
   sev.sigev_value.sival_ptr = node;
   timer_create(CLOCK_REALTIME, &sev, &node->timerid);

   // Set the timer interval and start the timer
   its.it_value.tv_sec = TIMER_INTERVAL_SEC;
   its.it_value.tv_nsec = TIMER_INTERVAL_NSEC;
   its.it_interval.tv_sec = 0;
   its.it_interval.tv_nsec = 0;
   timer_settime(node->timerid, 0, &its, NULL);
}

/**
 * @brief Set the Alive variable in Node struct
 * but also if not alive, set close state to -1(black)
 * @param id 
 * @param isAlive 
 */
void SetAlive(Node* node, bool isAlive)
{
   node->isAlive = isAlive;
   if(!isAlive)
   {
      sync_data d = {.id = node->id, .closeState = -1, .uClose = true, .uTemp = false,
                     .uActive = false, .uVolt = false};
      MakeChangeLog(&d, 1);
      //list[node->id].closeState = -1;
   }
}
void RestartTimer(timer_t timerid)
{
   struct itimerspec its;
   its.it_value.tv_sec = TIMER_INTERVAL_SEC;
   its.it_value.tv_nsec = TIMER_INTERVAL_NSEC;
   its.it_interval.tv_sec = 0;
   its.it_interval.tv_nsec = 0;
   timer_settime(timerid, 0, &its, NULL);
   printf("restart timer\n");
}
/**
 * @brief called when CMD_HEARTBEAT is received,   if counter == 0 add id.
 * search for id,if found restart timer ,
 *  if not found add id to list 
 * @param id int
 */
void HeartbeatHandler(uint8_t id, strip_t* childsStrip)
{
    //add all nodes from this child to own monitoring head
    //AddNodeToStrip checkt if id is already present
   printf("id; %d; len = %d\n", id, childsStrip->lenChildArr);
   fflush(stdout);
   for(uint8_t i = 0; i < childsStrip->lenChildArr; i++)
      monitoring_head = AddNodeToStrip(monitoring_head, childsStrip->childArr[i]);
        
   //if list is from other server, check for every node in childlist
   if(id == 255 || id == 254)
   {
      for(uint8_t i = 0; i < monitoring_head->lenChildArr; i++)
      {
         for(uint8_t j =0 ; j < childsStrip->lenChildArr; j++)
         {
            //check childs number in monitoring_head
            if(childsStrip->childArr[j]->id == monitoring_head->childArr[i]->id)
            {
               //set isAlive & restart timer
               monitoring_head->childArr[i]->isAlive = childsStrip->childArr[j]->isAlive;
               //don't call SetAlive others make changelog, not needed
               if(monitoring_head->childArr[i]->isAlive == false)
                  list[monitoring_head->childArr[i]->id].closeState = Err;
               printf("%s; id = %d, isAlive = %d", __func__, monitoring_head->childArr[i]->id,
                  monitoring_head->childArr[i]->isAlive);
               
               RestartTimer(monitoring_head->childArr[i]->timerid);
            }
         }
      }
   }
   else //normal node is sending, so only reset incoming id in monitoring list
   {
      for(uint8_t i = 0;i < monitoring_head->lenChildArr; i++)
      {
         if(monitoring_head->childArr[i]->id == id)
         {
            RestartTimer(monitoring_head->childArr[i]->timerid);
            SetAlive(monitoring_head->childArr[i], true);
            //monitoring_head->childArr[i]->isAlive = true
         }
      }
      //send to other server
      char tx_buf[2000];
      fflush(stdout);
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
      
    
   
   //debug
   DisplayMonitoringString();
   return;
}