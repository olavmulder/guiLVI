#include "../include/socket.h"
#include <pthread.h>
static int opt = true;  
static int master_socket , addrlen , new_socket , client_socket[10] , 
      max_clients = MAX_CLIENTS , activity, i , sd;  
static int max_sd;  
static fd_set readfds; 
static struct sockaddr_in server_addr;
volatile bool _type_sock;
int port_;
int clientSyncSocketNumber = -1;
static int client_fd;
struct sockaddr_in client_addr;
char ownAddr_[20];
sem_t mutex;


void* waitT()
{
   usleep(500);
}
//server tcp/ip functions
int _ServerInitTCP_IP_Multi(char* ownAddr)
{
   sem_init(&mutex, 0,1);

   strcpy(ownAddr_, ownAddr);
   //initialise all client_socket[] to 0 so not checked 
   for (i = 0; i < max_clients; i++)  
   {  
      client_socket[i] = 0;  
   }  
            
   //create a master socket 
   if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
   {  
      perror("socket failed");  
      return -1;
   }  

   //set master socket to allow multiple connections , 
   //this is just a good habit, it will work without this 
   if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
            sizeof(opt)) < 0 )  
   {  
      perror("setsockopt");  
      return -1;
   }  

   // Bind the socket to the interface
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(PORT);
   inet_pton(AF_INET, ownAddr, &(server_addr.sin_addr));
   //server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            
   //bind the socket to localhost port 
   if (bind(master_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)  
   {  
      perror("bind failed");  
      return -1;
   }  
   printf("Listener;port %d \n", PORT);  
            
   //try to specify maximum of 3 pending connections for the master socket 
   if (listen(master_socket, 3) < 0)  
   {  
      perror("listen");  
      return -1;
   }      
   //accept the incoming connection 
   addrlen = sizeof(server_addr);  
   puts("Waiting for connections ...");  
   _type_sock = 0;
   return 1;
}
int _HandleIncommingConnection()
{
   //If something happened on the master socket , 
   //then its an incoming connection 
   if (FD_ISSET(master_socket, &readfds))  
   {  
      if ((new_socket = accept(master_socket, 
            (struct sockaddr *)&server_addr, (socklen_t*)&addrlen))<0)  
      {  
         perror("accept");  
         return -1;
      } 
      if(strcmp(inet_ntoa(server_addr.sin_addr), ownAddr_ ) == 0)
      {
         printf("own addr, close socket");
         close(new_socket);
      }
      //inform user of socket number - used in send and receive commands 
      printf("New connection , socket fd is %d , ip is : %s , port : %d\
            \n" , new_socket , inet_ntoa(server_addr.sin_addr) , ntohs
            (server_addr.sin_port));  
      
      /*if( send(new_socket, "hoi", strlen("hoi"), 0) != strlen("hoi") )  
      {  
            perror("send");  
      } */
      //add new socket to array of sockets 
      for (i = 0; i < max_clients; i++)  
      {  
         //if position is empty 
         if( client_socket[i] == 0 )  
         {  
            client_socket[i] = new_socket;  
            printf("Adding to list of sockets as %d\n" , i);          
            break;  
         }  
      }  
   }  
   return 0;
}
void _HandleSocket()
{
   //clear the socket set 
   //printf("%s", __func__);
   fflush(stdout);
   FD_ZERO(&readfds);  
   //add master socket to set 
   FD_SET(master_socket, &readfds);  
   max_sd = master_socket;    
   //add child sockets to set 
   for ( i = 0 ; i < max_clients ; i++)  
   {  
      //socket descriptor 
      sd = client_socket[i];  
            
      //if valid socket descriptor then add to read list 
      if(sd > 0)  
      FD_SET( sd , &readfds);  
            
      //highest file descriptor number, need it for the select function 
      if(sd > max_sd)  
      max_sd = sd;  
   }  
   //printf("%s end", __func__);
   fflush(stdout);
}
void _GetSocketActivity()
{
   //wait for an activity on one of the sockets , timeout is NULL , 
   //so wait indefinitely 
   //printf("%s", __func__);
   fflush(stdout);
   struct timeval tv = {.tv_sec = 0, .tv_usec =50};
   activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);  

   if ((activity < 0) && (errno!=EINTR))  
   {  
      printf("select error");  
   }  

}
void _HandleActivity(int i)
{

   //printf("%s", __func__);
   fflush(stdout);
   char buffer[1025];  //data buffer of 1K 
   //Check if it was for closing , and also read the 
   //incoming message 
   ssize_t valread;
   if ((valread = read( sd , buffer, 1024)) == 0)  
   {  
      //Somebody disconnected , get his details and print 
      getpeername(sd , (struct sockaddr*)&server_addr , \
      (socklen_t*)&addrlen);  
      printf("Host disconnected , ip %s , port %d \n" , 
            inet_ntoa(server_addr.sin_addr) , ntohs(server_addr.sin_port));  
      fflush(stdout);
      //Close the socket and mark as 0 in list for reuse 
      close( sd );  
      client_socket[i] = 0;  
   }  
            
   //send message back
   else 
   {  
      //printf("received on server\n");
      //fflush(stdout);
      getpeername(sd , (struct sockaddr*)&server_addr , \
      (socklen_t*)&addrlen);  
      if(strcmp (inet_ntoa(server_addr.sin_addr) , ownAddr_ ) == 0)
      {
         printf("received from same ip\n");
         return;
      }
      int res;
      mesh_data ret;
      //printf("received,ip %s , port %d; %s\n",
      //      inet_ntoa(server_addr.sin_addr) , ntohs(server_addr.sin_port), buffer);
      //fflush(stdout);
      //sem_wait(&mutex);
         res = HandleIncomingData(&ret, buffer, strlen(buffer));
      //sem_post(&mutex);
      if(res == 1)
      {
         //printf("received sync init\n");
         fflush(stdout);
         clientSyncSocketNumber = i;
         //init send
         //sleep(1);
         //SendCopyData();
      }
      else if(res == 0)
      {
         if(ret.cmd == CMD_SYNC)
            updateFlag = 1;

         //SendToLvi
         char buf[2000];
         //only return when the CMD type expects a return msg(utils.h)
         if(is_CMD_a_Return_Msg(ret.cmd)) 
         {
            _MakeMsgLvi(&ret, buf, sizeof(buf));
            send(sd, buf, strlen(buf), MSG_CONFIRM);
         }
         
      }
      
      if(res < 0)
      {
         printf("%s;error handleIncomingdata: %d\n", __func__, res);
         return;
      }
   }
  
}

int ServerLoopTCP_IP_MULTI(char* ownAddr)
{   
   
   if(_ServerInitTCP_IP_Multi(ownAddr) < 0)return -1;

   //set of socket descriptors
   while(true)  
   {  
      _HandleSocket();
      _GetSocketActivity();     
      _HandleIncommingConnection();
     
      //else its some IO operation on some other socket
      for (i = 0; i < max_clients; i++)  
      {  
         sd = client_socket[i];  
                  
         if (FD_ISSET( sd , &readfds))  
         {  
            _HandleActivity(i);
         }  
      }
   }  
   
         
   return 0;  
}  


bool _ClientInit(char* ownAddr, char* serverAddr){
   int status;
   int opt = 1;
   //printf("client init");
   if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      printf("\n Socket creation error \n");
      return false;
   }

   if (setsockopt(client_fd, SOL_SOCKET, 
                  SO_REUSEADDR | SO_REUSEPORT, &opt,
                  sizeof(opt))) {
      perror("setsockopt");
      return false;
   }
   /*if (setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO
                  , &tv,
                  sizeof(tv))) {
      perror("setsockopt 2");
      return -1;
   }*/
   // Set up client address
   struct sockaddr_in client_address;
   memset(&client_address, 0, sizeof(client_address));
   client_address.sin_family = AF_INET;
   client_address.sin_port = htons(0); // Select an ephemeral port for the client socket
   //char* ownAddr = "192.168.178.25";
   if (inet_pton(AF_INET, ownAddr, &(client_address.sin_addr)) <= 0) {
      perror("Invalid address/Address not supported\n");
      exit(1);
   }

   // Bind the socket to the client address
   if (bind(client_fd, (struct sockaddr*)&client_address, sizeof(client_address)) < 0) {
      perror("Bind failed");
      exit(1);
   }

   // Set up server address
   struct sockaddr_in server_address;
   memset(&server_address, 0, sizeof(server_address));
   server_address.sin_family = AF_INET;
   server_address.sin_port = htons(PORT); //Port number
   server_address.sin_addr.s_addr = inet_addr(serverAddr);
   /*char* serverAddr = "192.168.2.103";
   if (inet_pton(AF_INET, serverAddr, &(server_address.sin_addr)) <= 0) {
      perror("Invalid address/Address not supported\n");
      exit(1);
   }*/

   if ((status
      = connect(client_fd, (struct sockaddr*)&server_address,
                  sizeof(server_address))) < 0) 
   {
      printf("\nConnection Failed \n");
      return false;
      //there is no server -> become server
   }
   printf("init as Client, to %s: %d\n", serverAddr, PORT);
   fflush(stdout);
   InitMsg();
   //AskCopyData();
   return true;
}

int SendSync(const char *msg, size_t len)
{
   ssize_t res;
   //printf("%s, send sync %s\n", __func__, msg);
   //if(_type_sock){
      //#ifdef C
      if(clientSyncSocketNumber == -1)
      {
         if(client_fd == 0)return -1;
         res =  send(client_fd, msg, len+1, MSG_CONFIRM);
         //printf("%s, send cleint_fd res: %ld\n", __func__,res);
      }
      else
      {
      //#else
         res =  send(client_socket[clientSyncSocketNumber], msg, len+1, MSG_CONFIRM);
         //printf("%s, send client_socket[%d] res: %ld\n", __func__,clientSyncSocketNumber,res);
      }
      //#endif
   //}
   /*else
   {
      res =  send(client_socket[clientSyncSocketNumber], msg, len, 0);
      printf("%s, type socket false;res: %ld", __func__,res);

   }*/
   if(res <= 0)
   {
      fprintf(stderr, "%s;res: %ld\n", __func__,res);
      //while(_ClientInit(port_) != true)close(client_fd);
   }
   fflush(stdout);

   return (int)res;
}
/**
 * @brief setup client,
 * if init is successfull ask for copy data.
 * client loop used for sync alg
 * @return int -1 on failure 0 on OK
 */
void ClientLoop(char* ownAddr, char* serverAddr)
{
   ssize_t valread;
   while(!_ClientInit(ownAddr, serverAddr));
   //always receive;
   char buffer_rx[BUF_RX_SIZE];
   while(1)
   {  
      valread = -1;
      valread = read(client_fd, buffer_rx, BUF_RX_SIZE);
      //valread = recv(client_fd, buffer_rx, BUF_RX_SIZE, MSG_DONTWAIT);
      if(valread > 0)
      {
         //printf("CLIENT RECEIVE\n");
         //printf("buffer: %s", buffer_rx);
         mesh_data r;
         //sem_wait(&mutex);
            int res = HandleIncomingData(&r, buffer_rx, strlen(buffer_rx));
            if(r.cmd == CMD_SYNC)
               updateFlag = 1;
         //sem_wait(&mutex);
         
         //updateFlag = 1;
         if(res < 0){
            fprintf(stderr, "HandleIncomingData error %d\n", res);
            //char buf[50];
            //snprintf(buf, 50, "{\n\"error\":%d\n}", res);
            //send(client_fd, buf, strlen(buf), MSG_CONFIRM);
         }else
            memset(buffer_rx, 0, BUF_RX_SIZE);
      }
      pthread_t wait;
      pthread_create(&wait, NULL, waitT, NULL);
      pthread_join(wait, NULL);
   }
}

