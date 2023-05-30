#include "../include/heartbeat.h"

#define TIMER_INTERVAL_SEC 10
#define TIMER_INTERVAL_NSEC 0

strip_t *monitoring_head;

void TimerHandler(int sig, siginfo_t *si, void *uc)
{
   Node* data = (Node*)(si->si_value.sival_ptr);
   printf("timer expired of id:%d\n",data->id);
   printf("timer addr %p\n", data);
   RestartTimer(data->timerid); 
   SetAlive(data, false);
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
      MakeChangeLog(node->id, 0, -1, 0, false,false, true, false, false);
      //list[node->id].closeState = -1;
   }
}
void RestartTimer(timer_t timerid)
{
   printf("restart timer\n");
   struct itimerspec its;
   its.it_value.tv_sec = TIMER_INTERVAL_SEC;
   its.it_value.tv_nsec = TIMER_INTERVAL_NSEC;
   its.it_interval.tv_sec = 0;
   its.it_interval.tv_nsec = 0;
   timer_settime(timerid, 0, &its, NULL);
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
   fflush(stdout);
   for(uint8_t i = 0; i < childsStrip->lenChildArr; i++)
      monitoring_head = AddNodeToStrip(monitoring_head, childsStrip->childArr[i]);
        
    //if not empty search node in strip
   //printf("%s; len = %d\n", __func__, monitoring_head->lenChildArr);
   for(uint8_t i = 0;i < monitoring_head->lenChildArr; i++)
   {
      //if found restart timer and return
      if(monitoring_head->childArr[i]->id == id)
      {
        // printf("id %d ", monitoring_head->childArr[i]->id);
        // printf("timer addr %p ", monitoring_head->childArr[i]);

         RestartTimer(monitoring_head->childArr[i]->timerid);
         SetAlive(monitoring_head->childArr[i], true);
         //monitoring_head->childArr[i]->isAlive = true
         fflush(stdout);
      }
   }
   //debug
   DisplayMonitoringString();
   return;
}