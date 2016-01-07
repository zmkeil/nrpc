
/***********************************************
  File name		: controller.cpp
  Create date	: 2015-12-14 01:16
  Modified date : 2016-01-07 23:34
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "timer.h"
#include "controller.h"
#include "service_context_log.h"
#include "proto/nrpc_meta.pb.h"

namespace nrpc
{

Controller::Controller() :
    _result(RPC_OK),
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
 * for client side
 *    about _state and ErrorText
 **************************************/
bool Controller::Failed() const
{
    // 1.the _state is inited to be RPC_OK, so this call returns false before
    // rpc_over in async thread as long as no error occurs
    // 2.RPC_OK means rpc frame success, then check resp_meta->error_code
    if (_result == RPC_OK) {
        RpcMeta* rpc_meta = ((DefaultProtocolCtx*)_protocol_ctx)->rpc_meta;
        if (rpc_meta->has_response() &&
                rpc_meta->response().has_error_code() &&
                (rpc_meta->response().error_code() != RPC_SERVICE_OK)) {
            return true;
        }
        return false;
    }
    return true;
}

std::string Controller::ErrorText() const
{
    // before (and in) protocol->process_response (in other words, in rpc frame),
    // return the _result_text; otherwise, the rpc procedure is successly completed,
    // than get error_code and error_text from rpc_meta->response
    if (_result != RPC_OK) {
        return std::string(_result_text);
    }

    RpcMeta* rpc_meta = ((DefaultProtocolCtx*)_protocol_ctx)->rpc_meta;
    if (rpc_meta->has_response()) {
        const RpcResponseMeta& resp_meta = rpc_meta->response();
        if (resp_meta.has_error_text()) {
            return resp_meta.error_text();
        }
        if (resp_meta.has_error_code() &&
                (resp_meta.error_code() != RPC_SERVICE_OK)) {
            return std::string("Unkown rpc service error");
        }
    }
    return std::string("OK");
}


/***************************************
 * for server side
 *    about service and server option
 **************************************/
bool Controller::server_side_init(ngx_connection_t* c)
{
    _start_time_s = ngxplus::Timer::rawtime();
    _start_time_us = ngxplus::Timer::rawtime_us();
    _ngx_connection = c;
    _service_set = (ServiceSet*)_ngx_connection->listening->servers;
    _server = _service_set->server();
    _service_context = _server->local_service_context();
    // in server side, start the session with READING REQUEST
    _state = RPC_SESSION_READING;
    // cmp c->local_sockaddr with service_set.address
/*     if (not equal) {
 *         rpc_session->set_result(RPC_INNER_ERROR);
 *         rpc_session->set_result_text("request ip not bind to this port");
 *         return false;
 *     }
 */
    return true;
}

void Controller::SetFailed(const std::string& reason) {
    RpcMeta* rpc_meta = ((DefaultProtocolCtx*)_protocol_ctx)->rpc_meta;
    RpcResponseMeta* resp_meta = rpc_meta->mutable_response();
    resp_meta->set_error_code(RPC_SERVICE_FAILED);
    resp_meta->set_error_text(reason);
    return;
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
    // free iobuf
    delete _iobuf;

    // log
    ServiceContextLog* access_log = ServiceContextLog::get_context();
    access_log->clear();
    access_log->set_start_time(_start_time_s);
    access_log->set_rt(_end_time_us - _start_time_us);
    access_log->set_ret_code(_result);
    access_log->set_ret_text(_state, _result_text);
    // the _service_context alloc in init(), some field will be null if errors occurs
    if (_service_context) {
        access_log->push_service_context(_service_context);
    }
    access_log->flush();

    // close connection, clear timer and event OR keepalive for reuse
    // finalize_server_connection(_ngx_connection);

    return;
}

}
