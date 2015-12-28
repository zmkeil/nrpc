
/***********************************************
  File name		: rpc_session.h
  Create date	: 2015-12-02 23:47
  Modified date : 2015-12-29 00:17
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#ifndef NRPC_RPC_SESSION_H
#define NRPC_RPC_SESSION_H

extern "C" {
#include <nginx.h>
#include <ngx_core.h>
}
#include "service_set.h"
#include "server.h"

void ngx_nrpc_init_connection(ngx_connection_t *c);

namespace nrpc
{

class RpcSession
{
public:
    int protocol_type;
    int client;
    ServiceSet* service_set;
    Server* server;
};

}

#endif
