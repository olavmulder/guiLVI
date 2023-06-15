#include "../include/socket.h"
#include <pthread.h>

typedef struct _client_socket
{
   int socket_fd;
   int lvi_id;
}Client_socket;

static int opt = true;  
static int master_socket , addrlen , new_socket , 
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


Client_socket client_socket[10] = {0};

void* waitT()
{
   usleep(500);
}
//server tcp/ip functions
int _ServerInitTCP_IP_Multi(char* ownAddr)
{
   strcpy(ownAddr_, ownAddr);
   //initialise all client_socket[] to 0 so not checked 
   for (i = 0; i < max_clients; i++)  
   {  
      client_socket[i].socket_fd = 0;  
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

      //add new socket to array of sockets 
      for (i = 0; i < max_clients; i++)  
      {  
         //if position is empty 
         if( client_socket[i].socket_fd == 0 )  
         {  
            client_socket[i].socket_fd = new_socket;

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
      sd = client_socket[i].socket_fd;  
            
      //if valid socket descriptor then add to read list 
      if(sd > 0)  
      FD_SET( sd , &readfds);  
            
      //highest file descriptor number, need it for the select function 
      if(sd > max_sd)  
      max_sd = sd;  
   }  

}
void _GetSocketActivity()
{
   //wait for an activity on one of the sockets , timeout is NULL , 
   //so wait indefinitely 
   struct timeval tv = {.tv_sec = 0, .tv_usec =50};
   activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);  

   if ((activity < 0) && (errno!=EINTR))  
   {  
      printf("select error");  
   }  
}
//close sync client & reset values
/*
void CloseSyncSocket()
{
   if(clientSyncSocketNumber >= 0)
   {
      if(client_socket[clientSyncSocketNumber].socket_fd > 0)
      {
         close(client_socket[clientSyncSocketNumber].socket_fd);
         client_socket[clientSyncSocketNumber].socket_fd = 0;
         client_socket[clientSyncSocketNumber].lvi_id = -1;
         clientSyncSocketNumber  = -1;
      }
   }
   else //i'm a client
   {
      close(client_fd);
      client_fd = -1;
   }  
}*/
void _HandleActivity(int i)
{
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
      client_socket[i].socket_fd = 0; 
   }  
            
   //send message back
   else 
   {  
      getpeername(sd , (struct sockaddr*)&server_addr , \
      (socklen_t*)&addrlen);  
      if(strcmp (inet_ntoa(server_addr.sin_addr) , ownAddr_ ) == 0)
      {
         printf("received from same ip\n");
         return;
      }
      int res;
      mesh_data ret;
    
      res = HandleIncomingData(&ret, buffer, strlen(buffer));
      if(res == 1)
      {
         printf("received sync init\n");// comes from cliet other pc
         fflush(stdout);
         clientSyncSocketNumber = i;

         char* buf = SendCopyData();
         
         SendSync(buf, strlen(buf));
         free(buf);
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
            if(ret.cmd == CMD_HEARTBEAT)
               ret.cmd = CMD_HEARTBEAT_CONFIRM;
            _MakeMsgLvi(&ret, buf, sizeof(buf));
            if(ret.cmd == CMD_HEARTBEAT_CONFIRM)
               printf("confirm: %s\n", buf);
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
         sd = client_socket[i].socket_fd;  
                  
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
   if(client_fd) close(client_fd);
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
  
   // Set up client address
   struct sockaddr_in client_address;
   memset(&client_address, 0, sizeof(client_address));
   client_address.sin_family = AF_INET;
   client_address.sin_port = htons(0); // Select an ephemeral port for the client socket
   //char* ownAddr = "192.168.178.25";
   if (inet_pton(AF_INET, ownAddr, &(client_address.sin_addr)) <= 0) {
      perror("Invalid address/Address not supported\n");
      return false;
   }

   // Bind the socket to the client address
   if (bind(client_fd, (struct sockaddr*)&client_address, sizeof(client_address)) < 0) {
      perror("Bind failed");
      return false;
   }

   // Set up server address
   struct sockaddr_in server_address;
   memset(&server_address, 0, sizeof(server_address));
   server_address.sin_family = AF_INET;
   server_address.sin_port = htons(PORT); //Port number
   server_address.sin_addr.s_addr = inet_addr(serverAddr);

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
         if(res <=0)
         {
            printf("%s, send cleint_fd res: %ld\n", __func__,res);
            #ifdef C
            _ClientInit(ownAddr_,SERVER_IP[0]);
            #endif
         }
      }
      else
      {
         res =  send(client_socket[clientSyncSocketNumber].socket_fd, msg, len+1, MSG_CONFIRM);
         if(res <= 0)
            printf("%s, send client_socket[%d] res: %ld\n", __func__,clientSyncSocketNumber,res);
      }

   if(res <= 0)
   {
      fprintf(stderr, "%s;res: %ld\n", __func__,res);
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
      if(client_fd == -1)_ClientInit(ownAddr, serverAddr);
      else
      {
         valread = -1;
         valread = read(client_fd, buffer_rx, BUF_RX_SIZE);
         if(valread > 0)
         {
            mesh_data r;  
            int res = HandleIncomingData(&r, buffer_rx, strlen(buffer_rx));
            if(r.cmd == CMD_SYNC)
               updateFlag = 1;
            if(res < 0){
               fprintf(stderr, "HandleIncomingData error %d\n", res);
            }
            else
               memset(buffer_rx, 0, BUF_RX_SIZE);
         }
      }
      pthread_t wait;
      pthread_create(&wait, NULL, waitT, NULL);
      pthread_join(wait, NULL);
   }
}