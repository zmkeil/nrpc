#ifndef  NRPC_NGX_NRPC_HANDLER_H
#define  NRPC_NGX_NRPC_HANDLER_H

extern "C" {
#include <ngx_event.h>
#include <ngx_core.h>
}
#include "rpc_session.h"

namespace nrpc {

// request
void ngx_nrpc_init_connection(ngx_connection_t *c);

void ngx_nrpc_determine_policy(ngx_event_t *rev);

void ngx_nrpc_read_request(ngx_event_t *rev);

void ngx_nrpc_dummy_read(ngx_event_t* rev);

// response
void ngx_nrpc_send_response(ngx_event_t* wev);

}
#endif

