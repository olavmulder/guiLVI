#include "../include/strip.h"
const char* TAG_STRIP = "STRIP:";
/**
 * @brief make new node, init it with data and return it
 * 
 * @param id 
 * @param ip 
 * @param mac 
 * @param port 
 * @param isAlive 
 * @return Node* 
 */
Node* NewNode(int id, char* ip, char *mac, uint16_t port, bool isAlive)//data
{
   Node *temp = (Node*)malloc(sizeof(Node));
   temp->id = id;
   strcpy(temp->ip_wifi,ip);
   strcpy(temp->mac_wifi,mac);
   temp->port = port;
   temp->isAlive = isAlive;
   InitTimer(temp);
   return temp;
}
/**
 * @brief find id in monitoring linked list
 * 
 * @param id 
 * @return int -1 not found, int index of found
 */
int FindID(int id)
{
    //ESP_LOGI(TAG_STRIP, "%s, id = %d", __func__, id);
    int max = monitoring_head->lenChildArr;
    int index = 0;
        while(index < max)
    {
      if(monitoring_head->childArr[index]->id == id)
        return index;
      index++;
    }
   return -1;
}
strip_t* RemoveFromStrip(strip_t* strip, int id)
{
    for(uint8_t i = 0; i < strip->lenChildArr; i++)
    {
        if(strip->childArr[i]->id == id)
        {
            //remove
            for(uint8_t j = i; j < strip->lenChildArr - 1; j++)
            {
                timer_delete(strip->childArr[i]->timerid);
                memcpy(strip->childArr[i], strip->childArr[i + 1], sizeof(Node));

            }
            free(strip->childArr[strip->lenChildArr-1]);
            strip->lenChildArr -= 1;
            return strip;
        }
    }
    return strip;
}
/**
 * @brief add node to node strip when it isn't there already
 * , and init timer
 * 
 * @param strip 
 * @param n 
 * @return strip_t* 
 */
strip_t* AddNodeToStrip(strip_t* strip, Node* n)
{
    uint8_t inID = n->id;
    //only add new node if id of incoming node is not in monitoring_head
    int res = FindID(inID);
    if(res == -1)
    {
        //init timeouts
        n->timeouts = 0;
        if(strip->lenChildArr ==0)
        {
            strip->childArr[0] = (Node*)malloc(sizeof(Node));
            memcpy(strip->childArr[0],n, sizeof(Node));
            strip->lenChildArr = 1;
        }
        else
        {
            strip->childArr = realloc(strip->childArr, sizeof(Node*) * ++strip->lenChildArr);
            strip->childArr[strip->lenChildArr-1] = (Node*)malloc(sizeof(Node));
            memcpy(strip->childArr[strip->lenChildArr-1], n, sizeof(Node));
        }
        InitTimer(strip->childArr[strip->lenChildArr-1]);

    }
    /*else{
        //if incoming node is true and in my strip it is not alive, set it as alive
        if(n->isAlive == true)
        {
            for(uint8_t i = 0; i < strip->lenChildArr;i++)
            {
                if(strip->childArr[i]->id == inID)
                {
                    if(strip->childArr[i]->isAlive == false)
                        strip->childArr[i]->isAlive  = true;
                }
            }
        }
    }*/
    return strip;
}
/**
 * @brief show what is in monitoring_head
 * 
 */
void DisplayMonitoringString()
{
   printf("%s:\n",__func__);
   
    for(uint8_t i = 0; i < monitoring_head->lenChildArr;i++)
    {
         printf(" id:%d, isAlive: %d, ip:%s\n", monitoring_head->childArr[i]->id,
                monitoring_head->childArr[i]->isAlive, monitoring_head->childArr[i]->ip_wifi);
    }
}