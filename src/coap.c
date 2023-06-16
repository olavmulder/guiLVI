#include "../include/coap.h"

//#include <pthread.h>

volatile int have_response = 0;

mesh_data nodes[200];
static int syncServerID = -1;

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
        char retDataString[1000];
        //coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
        printf("coap received");
        fflush(stdout);
        mesh_data r;
        HandleIncomingData(&r, (char*)data, len+1);
        if(r.cmd == CMD_HEARTBEAT)
        {
                r.cmd = CMD_HEARTBEAT_CONFIRM;
                _MakeMsgLvi(&r, retDataString, sizeof(retDataString));
                if(r.cmd == CMD_HEARTBEAT_CONFIRM)
                        printf("confirm: %s\n", retDataString);
                coap_add_data(response, strlen(retDataString),
                                (const uint8_t *)retDataString);
        }
        else if(r.cmd == CMD_HEARTBEAT_CONFIRM)
        {
                memcpy(r.mac, "aa:bb:cc", sizeof(r.mac));
                r.id = 255;
                memcpy(r.ip, ownAddr_, sizeof(r.ip));
                r.port = 8080;
                _MakeMsgLvi(&r, retDataString, sizeof(retDataString));
                coap_add_data(response, strlen(retDataString),
                                (const uint8_t *)retDataString);
        }
        //only make return msg when cmd == init , hearbeat or to_client
        else if(is_CMD_a_Return_Msg(r.cmd))
        {

                if(_MakeMsgLvi(&r, retDataString, sizeof(retDataString))  < 0)
                {
                        goto end;
                }
                coap_add_data(response, strlen(retDataString),
                                (const uint8_t *)retDataString);
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