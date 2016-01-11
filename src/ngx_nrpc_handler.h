#ifndef  NRPC_NGX_NRPC_HANDLER_H
#define  NRPC_NGX_NRPC_HANDLER_H

extern "C" {
#include <ngx_event.h>
#include <ngx_core.h>
}

namespace nrpc {

// request
void ngx_nrpc_init_connection(ngx_connection_t *c);

void ngx_nrpc_determine_policy(ngx_event_t *rev);

void ngx_nrpc_read_request(ngx_event_t *rev);

void ngx_nrpc_dummy_read(ngx_event_t* rev);

// response
void ngx_nrpc_send_response(ngx_event_t* wev);

void ngx_nrpc_dummy_write(ngx_event_t* wev);

// finalize
void ngx_nrpc_close_connection(ngx_connection_t* c);

// reuse
void ngx_nrpc_reuse_connection(ngx_event_t *rev);

}
#endif

