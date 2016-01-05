
/***********************************************
  File name		: controller.cpp
  Create date	: 2015-12-14 01:16
  Modified date : 2016-01-05 00:50
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "timer.h"
#include "controller.h"
#include "service_context_log.h"

namespace nrpc
{

Controller::Controller() :
    _result_text(nullptr),
    _protocol(nullptr),
    _protocol_ctx(nullptr),
    _request(nullptr),
    _response(nullptr),
    _iobuf(nullptr),
    _service_set(nullptr),
    _server(nullptr),
    _service_context(nullptr),
    _ngx_connection(nullptr)
{
}

Controller::~Controller()
{
}

/***************************************
 * for server side
 *    about service and server option
 **************************************/
bool Controller::server_side_init(ngx_connection_t* c)
{
    _ngx_connection = c;
    _service_set = (ServiceSet*)_ngx_connection->listening->servers;
    _server = _service_set->server();
    _service_context = _server->local_service_context();
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

// for server options
ServiceContext* Controller::service_context()
{
    return _server->local_service_context();
}

int Controller::server_read_timeout()
{
    return _server->read_timeout();
}


/***************************************
 * for both side
 *    about protocol and procedure
 **************************************/
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

void Controller::finalize()
{

    // server side
    // close connection, clear timer and event
    // finalize_server_connection(_ngx_connection);

    // free iobuf
    delete _iobuf;

    // log
    ServiceContextLog* access_log = ServiceContextLog::get_context();
    access_log->clear();
    access_log->set_start_time(10);
    access_log->set_rt(5);
    access_log->set_ret_code(_result);
    access_log->set_ret_text(_state, _result_text);
    // the _service_context alloc in init(), some field will be null if errors occurs
    if (_service_context) {
        access_log->push_service_context(_service_context);
    }

    return;
}

}
