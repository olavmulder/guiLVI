#include "../include/heartbeat.h"
#include "../include/data.h"

//add to monitoring list
/*int main()
{
   mesh_data ret;
   char incomming_message[] = "{\"id\":2,\"cmd\":6,\"mac\":\"78:21:84:a6:0c:84\",\"array\":[{\"id\":1,\"port\": 32808,\"ip\":\"48.198.247.38\",\"mac\":\"30:c6:f7:26:28:80\",\"isAlive\":0}]}";
   HandleIncomingData(&ret, incomming_message, strlen(incomming_message));
   assert(monitoring_head->lenChildArr == 2);
   assert(monitoring_head->childArr[0]->id ==2);
   assert(monitoring_head->childArr[1]->id ==1);
   assert(monitoring_head->childArr[0]->isAlive == true);
   sleep(11);
   assert(monitoring_head->childArr[0]->isAlive == false);
   assert(list[monitoring_head->childArr[0]->id].closeState == -1);
}*/

//test reset timer & steeds 1 lengte, want zelfde id
/*int main()
{
   mesh_data ret;
   char incomming_message[] = "{\"id\":2,\"cmd\":6,\"mac\":\"78:21:84:a6:0c:84\"}";
   HandleIncomingData(&ret, incomming_message, strlen(incomming_message));
   assert(monitoring_head->lenChildArr == 1);
   assert(monitoring_head->chilArr[0]->isAlive == true);
   sleep(5);
   HandleIncomingData(&ret, incomming_message, strlen(incomming_message));
   assert(monitoring_head->lenChildArr == 1);
   assert(monitoring_head->childArr[0]->isAlive == true);

   sleep(11);
   assert(monitoring_head->childArr[0]->isAlive == false);
   assert(list[monitoring_head->childArr[0]->id].closeState == -1);
}*/

//geen id = niks
/*int main()
{
   mesh_data ret;
   char incomming_message[] = "{\"cmd\":6,\"mac\":\"78:21:84:a6:0c:84\"}";
   assert(HandleIncomingData(&ret, incomming_message, strlen(incomming_message)) == -1);
}*/

//foutive input unitest
/*int main()
{
   //not a json format
   mesh_data ret;
   char message[] = "hoi"; 
   assert(HandleIncomingData(&ret,message, strlen(message)) == -1);
   //no cmd
   mesh_data ret2;
   char message2[500];
   snprintf(message2, 500, "{\"%s\" : 1}", nameID);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //invalid cmd
   snprintf(message2, 500, "{\"%s\" : %d}", nameCMD, 100);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   snprintf(message2, 500, "{\"%s\" : %d}", nameCMD, -1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   snprintf(message2, 500, "{\"%s\" : %d}", nameCMD, -1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //to server geen ip/mac/port/temp or id -> -1
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": \"%s\", \"%s\": %d, \"%s\": %.2f, \"%s\": %d}", 
   nameCMD, CMD_TO_SERVER, nameMAC, "aa:bb:cc", nameIP, "192.168.178.2", namePort, 1800, nameTemp, 0.00, nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == 0);
   //no mac
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\", \"%s\": %d, \"%s\": %.2f, \"%s\": %d}", 
   nameCMD, CMD_TO_SERVER,  nameIP, "192.168.178.2", namePort, 1800, nameTemp, 20.0, nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //no ip
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": %d, \"%s\": %.2f, \"%s\": %d}", 
   nameCMD, CMD_TO_SERVER, nameMAC, "aa:bb:cc", namePort, 1800, nameTemp, 20.0, nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //no port
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": \"%s\",\"%s\": %.2f, \"%s\": %d}", 
   nameCMD, CMD_TO_SERVER, nameMAC, "aa:bb:cc", nameIP, "192.168.178.2",  nameTemp, 20.0, nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //no temperature
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": \"%s\", \"%s\": %d, \"%s\": %d}", 
   nameCMD, CMD_TO_SERVER, nameMAC, "aa:bb:cc", nameIP, "192.168.178.2", namePort, 1800,  nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //no id
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": \"%s\", \"%s\": %d, \"%s\": %.2f}", 
   nameCMD, CMD_TO_SERVER, nameMAC, "aa:bb:cc", nameIP, "192.168.178.2", namePort, 1800, nameTemp, 20.0);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);

   //for CMD_INIT 
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": \"%s\", \"%s\": %d, \"%s\": %.2f, \"%s\": %d}", 
   nameCMD, CMD_INIT_SEND, nameMAC, "aa:bb:cc", nameIP, "192.168.178.2", namePort, 1800, nameTemp, 20.0, nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == 0);
   //no mac
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\", \"%s\": %d, \"%s\": %.2f, \"%s\": %d}", 
   nameCMD, CMD_INIT_SEND,  nameIP, "192.168.178.2", namePort, 1800, nameTemp, 20.0, nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //no ip
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": %d, \"%s\": %.2f, \"%s\": %d}", 
   nameCMD, CMD_INIT_SEND, nameMAC, "aa:bb:cc", namePort, 1800, nameTemp, 20.0, nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //no port
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": \"%s\",\"%s\": %.2f, \"%s\": %d}", 
   nameCMD, CMD_INIT_SEND, nameMAC, "aa:bb:cc", nameIP, "192.168.178.2",  nameTemp, 20.0, nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //no temperature
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": \"%s\", \"%s\": %d, \"%s\": %d}", 
   nameCMD, CMD_INIT_SEND, nameMAC, "aa:bb:cc", nameIP, "192.168.178.2", namePort, 1800,  nameID, 1);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   //no id
   snprintf(message2, 500, "{\"%s\" : %d, \"%s\": \"%s\",\"%s\": \"%s\", \"%s\": %d, \"%s\": %.2f}", 
   nameCMD, CMD_INIT_SEND, nameMAC, "aa:bb:cc", nameIP, "192.168.178.2", namePort, 1800, nameTemp, 20.0);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == -1);
   
   //if cmd = SYNC_INIT return 1
   snprintf(message2, 500, "{\"%s\" : %d}", 
   nameCMD, CMD_SYNC_INIT);
   assert(HandleIncomingData(&ret2,message2, strlen(message2)) == 1);

   //if CMD = heartbeat ->ID should be present
   mesh_data ret4;
   char message4[500];
   snprintf(message4, 500, "{\"%s\":%d,\"%s\":%s}", nameCMD, CMD_HEARTBEAT, nameMAC, "aa:bb:cc");
   assert(HandleIncomingData(&ret4,message4, strlen(message4)) < 0);

   snprintf(message4, 500, "{\"%s\":%d,\"%s\":1}", nameCMD, CMD_HEARTBEAT, nameID);
   assert(HandleIncomingData(&ret4,message4, strlen(message4)) == 0);

   //if cmd = sync id & counter should be present
   snprintf(message4, 500, "{\"%s\":%d}", nameCMD, CMD_SYNC);
   assert(HandleIncomingData(&ret4,message4, strlen(message4)) == -3);
   syncCounter = 1;
   //if counter to old return -2
   snprintf(message4, 500, "{\"%s\":%d, \"%s\": %d, \"%s\": %d}", nameCMD, CMD_SYNC, nameID, 1, nameCounter, 0);
   assert(HandleIncomingData(&ret4,message4, strlen(message4)) == -2);
   //no data to update return -1
   syncCounter =0;
   snprintf(message4, 500, "{\"%s\":%d, \"%s\": %d, \"%s\": %d}", nameCMD, CMD_SYNC, nameID, 1, nameCounter, 1);
   assert(HandleIncomingData(&ret4,message4, strlen(message4)) == -1);
   //update closestate
   list[1].closeState = 0;
   snprintf(message4, 500, "{\"%s\":%d, \"%s\": %d, \"%s\": %d, \"%s\": %d}", nameCMD, CMD_SYNC, nameID, 1, nameCounter, 2, nameCloseState, 2);
   assert(HandleIncomingData(&ret4,message4, strlen(message4)) == 0);
   assert(list[1].closeState == 2);
   
   printf("\nCompleted\n");
}
*/