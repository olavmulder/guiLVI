#include "../include/coap.h"

//#include <pthread.h>

volatile int have_response = 0;

mesh_data nodes[200];
static int syncServerID = -1;

/*
int _ServerSendToLvi(char* buffer_rx, size_t valread, int i)
{
        int ret = -1;
        char bufferReturn[500];
        //printf("received: %s", buffer_rx);
        //if received process incoming data in ReadDataCallback func.
        mesh_data r = _ReceiveFromLvi((const char*)buffer_rx, valread+1, i);
        if(r.cmd == CMD_SEND_ERR || r.cmd == CMD_HEARTBEAT){
                printf("RECEIVED CMD_SEND_ERR || cmd-heartbeat\n");
                goto end;
        }
        if(r.id > (int)(sizeof(list)/sizeof(DataList))) 
        {
                goto end;
        }

        r.voltageState = list[r.id].voltageState;
        r.closeState = list[r.id].closeState;
        r.temp = list[r.id].temp;

        if(_MakeMsgLvi(&r, bufferReturn, sizeof(bufferReturn)) == 0)
        {
                //printf("data to send:%s\n", bufferReturn);
                fflush(stdout);
                if (send(sd, bufferReturn, strlen(bufferReturn), 0) > 0)
                        ret = 0;
                else 
                        ret = -1;
                
        }else{
                //TODO send function to send data again or something.
                printf("error make msg lvi");
                ret = -1;
        }
        end:
        //reset buffer
        memset(buffer_rx, '\0', BUF_RX_SIZE);
        return ret;
        
}*/
void message_handler(
                coap_resource_t* rec,
                coap_session_t*s, 
                const coap_pdu_t *received,
                const coap_string_t* str,
                coap_pdu_t *response)
{

        size_t len;
        uint8_t*data;
        coap_get_data(received, &len, (const uint8_t**)&data);
        printf("data in %s\n", (char*)data);
        char retDataString[1000];
        //coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
        mesh_data r;
        HandleIncomingData(&r, (char*)data, len+1);
        printf("%s: r.cmd = %d\n", __func__, r.cmd);

        //only make return msg when cmd == init or to_client
        if(     r.cmd == CMD_INIT_INVALID       || 
                r.cmd == CMD_INIT_VALID         || 
                r.cmd == CMD_TO_CLIENT)
        {

                if(_MakeMsgLvi(&r, retDataString, sizeof(retDataString))  < 0)
                {
                        goto end;
                }
                coap_add_data(response, strlen(retDataString),
                                (const uint8_t *)retDataString);
                printf("%s: return: %s\n", __func__, retDataString);
                fflush(stdout);
        }
        end:
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
        fprintf(stderr, "send done\n");
        fflush(stderr);
}

/**
 * @brief send response message with states to lvi 
 * 
 * @param buffer_rx , incomming data
 * @param valread  , length incoming data
 * @return int -1 on any error, 0 on right sending data
 */


int ServerLoop(char* ipAddrServer, char* port, char* serverName)
{
        printf("start COAP server");
        fflush(stdout);
        coap_context_t  *ctx = NULL;
        coap_address_t dst;
        coap_resource_t *resource = NULL;
        coap_endpoint_t *endpoint = NULL;

        int result = EXIT_FAILURE;
        coap_str_const_t *ruri = coap_make_str_const(serverName);
        coap_startup();
        //resolve destination address where server should be sent 
        if(_CoapResolveAddress(ipAddrServer, port, &dst) < 0){
                coap_log(LOG_CRIT, "failed to resolve address\n");
                goto finish;
        }
        // create CoAP context and a client session 
        ctx = coap_new_context(NULL);
        if (!ctx || !(endpoint = coap_new_endpoint(ctx, &dst, COAP_PROTO_TCP))) {
                coap_log(LOG_EMERG, "cannot initialize context\n");
                goto finish;
        }
        coap_context_set_block_mode(ctx,
                COAP_BLOCK_USE_LIBCOAP | COAP_BLOCK_SINGLE_BODY);
        
        resource = coap_resource_init(ruri, 0);
        coap_register_handler(resource, COAP_REQUEST_GET, message_handler);

        coap_add_resource(ctx, resource);
        while (true) { coap_io_process(ctx, 3); }

        result = EXIT_SUCCESS;
        finish:

        coap_free_context(ctx);
        coap_cleanup();

        return result;
}

/**
 * @brief check if id is already init in initList, if not add info to array
 * else check if incoming id has the same ip & mac as last 
 * @param ret pointer of mesh_data, contains id, mac & ip 
 * @return void
 */
int _CoapResolveAddress(const char *host, const char *service, coap_address_t *dst)
{

  struct addrinfo *res, *ainfo;
  struct addrinfo hints;
  int error, len=-1;

  memset(&hints, 0, sizeof(hints));
  memset(dst, 0, sizeof(*dst));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family = AF_UNSPEC;

  error = getaddrinfo(host, service, &hints, &res);

  if (error != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return error;
  }

  for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
    switch (ainfo->ai_family) {
        case AF_INET6:
        case AF_INET:
                len = dst->size = (int)ainfo->ai_addrlen;
                memcpy(&dst->addr.sin6, ainfo->ai_addr, dst->size);
                goto finish;
        default:
        ;
        }
  }
  finish:
        freeaddrinfo(res);
        return len;
}

uint8_t GetClientID(){return clientId;}