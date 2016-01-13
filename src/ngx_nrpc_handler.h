#ifndef  NRPC_NGX_NRPC_HANDLER_H
#define  NRPC_NGX_NRPC_HANDLER_H

extern "C" {
#include <ngx_event.h>
#include <ngx_core.h>
}

namespace nrpc {

class Controller;

// request
void ngx_nrpc_init_connection(ngx_connection_t *c);

void ngx_nrpc_determine_policy(ngx_event_t *rev);

void ngx_nrpc_read_request(ngx_event_t *rev);

void ngx_nrpc_dummy_read(ngx_event_t* rev);

// response
void ngx_nrpc_send_response(ngx_event_t* wev);

void ngx_nrpc_dummy_write(ngx_event_t* wev);

// Server side finalize
// Standard finalize process for server side, this will run cntl->finalize()
// which will free resources of current session and determain whether or not to 
// reuse the connection
void ngx_nrpc_finalize_session(Controller* cntl);
// This will be called in two scene:
//   1.current cntl determain NOT reuse the connection 
//   2.reused connection timeout (here there is no new session)
//   3.recv FIN at the begin of session (for new or reused connection, just
//   close_connection and delete the empty_cntl, then return with a log)
void ngx_nrpc_close_connection(ngx_connection_t* c);

// reuse
void ngx_nrpc_reuse_connection(ngx_event_t *rev);

}
#endif

