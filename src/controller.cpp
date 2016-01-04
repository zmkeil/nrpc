
/***********************************************
  File name		: controller.cpp
  Create date	: 2015-12-14 01:16
  Modified date : 2016-01-05 00:50
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "controller.h"

namespace nrpc
{

Controller::Controller()
{
}

Controller::~Controller()
{
}

bool Controller::server_side_init(ngx_connection_t* c)
{
    _ngx_connection = c;
    _service_set = (ServiceSet*)_ngx_connection->listening->servers;
    _server = _service_set->server();
    // in server side, start the session with READING_REQUEST
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

void Controller::finalize()
{
    //LOG(NGX_LOG_LEVEL_ALERT, "%s", _result_text);
    return;
}

// set
bool Controller::set_state(RPC_SESSION_STATE state)
{
    _state = state;
    return true;
}

bool Controller::set_result(RPC_RESULT result)
{
    _result = result;
    return true;
}

bool Controller::set_result_text(const char* result_text)
{
    _result_text = result_text;
    return true;
}

bool Controller::set_protocol(unsigned protocol_num)
{
    if ((protocol_num >= sizeof(g_rpc_protocols)/sizeof(g_rpc_protocols[0])) ||
            (protocol_num != NRPC_PROTOCOL_DEFAULT_NUM)){
        _result = RPC_INNER_ERROR;
        _result_text = "protocol num not implement";
        return false; 
    }

    _protocol = g_rpc_protocols[protocol_num];
    _protocol_name = _protocol->name;
    _protocol_ctx = _protocol->ctx_factory->create_ctx();
    if (!_protocol_ctx) {
        _result = RPC_INNER_ERROR;
        _result_text = "new protocol_ctx error";
        return false;
    }
    return true;
}

bool Controller::set_iobuf(ngxplus::IOBuf* iobuf)
{
    _iobuf = iobuf;
    return true;
}

// get
ngxplus::IOBuf* Controller::iobuf()
{
    return _iobuf;
}

Protocol* Controller::protocol()
{
    return _protocol;
}

void* Controller::protocol_ctx()
{
    return _protocol_ctx;
}

RPC_SESSION_STATE Controller::state()
{
    return _state;
}

ServiceSet* Controller::service_set()
{
    return _service_set;
}

ngx_connection_t* Controller::connection()
{
    return _ngx_connection;
}


// for server options
int Controller::read_timeout()
{
    return _server->read_timeout();
}


// private
bool Controller::set_service_set(ServiceSet* service_set)
{
    _service_set = service_set;
    return true;
}

}
