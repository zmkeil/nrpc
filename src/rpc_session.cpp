
/***********************************************
  File name		: rpc_session.cpp
  Create date	: 2015-12-14 01:16
  Modified date : 2015-12-31 03:31
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "rpc_session.h"

namespace nrpc
{

RpcSession::RpcSession(ngx_connection_t* c) : _c(c)
{
}

RpcSession::~RpcSession()
{
}

bool RpcSession::init()
{
    _service_set = (ServiceSet*)_c->listening->servers;
    _server = _service_set->server();
    _state = RPC_SESSION_READING_REQUEST;
    // cmp c->local_sockaddr with service_set.address
/*     if (not equal) {
 *         rpc_session->set_result(RPC_INNER_ERROR);
 *         rpc_session->set_result_text("request ip not bind to this port");
 *         return false;
 *     }
 */
    return true;
}

void RpcSession::finalize()
{
    // log
    return;
}

// set
bool RpcSession::set_state(RPC_SESSION_STATE state)
{
    _state = state;
    return true;
}

bool RpcSession::set_result(RPC_RESULT result)
{
    _result = result;
    return true;
}

bool RpcSession::set_result_text(const char* result_text)
{
    _result_text = result_text;
    return true;
}

bool RpcSession::set_protocol(unsigned protocol_num)
{
    if ((protocol_num >= sizeof(g_rpc_protocols)) ||
            (protocol_num != 0/*default_protocol*/)){
        _result = RPC_INNER_ERROR;
        _result_text = "protocol num not implement";
        return false; 
    }

    _protocol = g_rpc_protocols[protocol_num];
    _protocol_name = _protocol->name;
    // TODO: how to reflact
    _protocol_ctx = new DefaultProtocolCtx();
    if (!_protocol_ctx) {
        _result = RPC_INNER_ERROR;
        _result_text = "new protocol_ctx error";
        return false;
    }
    return true;
}

bool RpcSession::set_iobuf(ngxplus::IOBuf* iobuf)
{
    _iobuf = iobuf;
    return true;
}

// get
ngxplus::IOBuf* RpcSession::iobuf()
{
    return _iobuf;
}

Protocol* RpcSession::protocol()
{
    return _protocol;
}

void* RpcSession::protocol_ctx()
{
    return _protocol_ctx;
}

RPC_SESSION_STATE RpcSession::state()
{
    return _state;
}

ServiceSet* RpcSession::service_set()
{
    return _service_set;
}

ngx_connection_t* RpcSession::connection()
{
    return _c;
}

int32_t RpcSession::process_error_code()
{
    return _process_error_code;
}

std::string RpcSession::process_error_text()
{
    return _process_error_text;
}

// for server options
int RpcSession::read_timeout()
{
    return _server->read_timeout();
}

}
