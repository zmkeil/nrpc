
/***********************************************
  File name		: log.cpp
  Create date	: 2015-12-17 23:28
  Modified date : 2015-12-18 01:47
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>
}

// nrpc error log
// before init, nginx use ngx_log_stderr()
// in our nrpc, just use std::cout simplely
//ngx_log_t* nrpc_log;


/* static u_char* ngx_prefix = (u_char*)"./";
 * bool nrpc_log_init()
 * {
 *     nrpc_log = ngx_log_init(ngx_prefix);
 *     if (!nrpc_log) {
 *         return false;
 *     }
 *     return true;
 * }
 */ 
