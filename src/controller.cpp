
/***********************************************
  File name		: controller.cpp
  Create date	: 2015-12-14 01:16
  Modified date : 2016-01-15 09:07
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "timer.h"
#include "controller.h"
#include "service_context_log.h"
#include "proto/nrpc_meta.pb.h"
#include "ngx_nrpc_handler.h"
#include "channel.h"
#include "connection_pool.h"

namespace nrpc
{

Controller::Controller() :
    _is_server(false),
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
    _ngx_connection(nullptr),
	_params(nullptr),
	_client_sockfd(-1),
	_client_recv_eof(false)
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
    // set _is_client false
    _is_server = true;
    // cmp c->local_sockaddr with service_set.address
/*     if (not equal) {
 *         rpc_session->set_result(RPC_INNER_ERROR);
 *         rpc_session->set_result_text("request ip not bind to this port");
 *         return false;
 *     }
 */
    return true;
}

void Controller::SetFailed(const std::string& reason)
{
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

int Controller::server_write_timeout()
{
    return _server->write_timeout();
}

bool Controller::get_concurrency()
{
	return _server->get_concurrency();
}

void Controller::free_concurrency()
{
	return _server->free_concurrency();
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

void Controller::finalize_server_connection(ngx_connection_t* c)
{
	// close the connection if any errors, avoid dirty data
	if (_result != RPC_OK) {
		return ngx_nrpc_close_connection(c);
	}

	// reuse flag default false, set it in ServerOption
    if (!_server->is_connection_reuse()) {
        return ngx_nrpc_close_connection(c);
    }

    // reuse
    c->read->handler = ngx_nrpc_reuse_connection;
    c->write->handler = ngx_nrpc_dummy_write;
    ngx_add_timer(c->read, _server->idle_timeout() * 1000);
    if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
        LOG(ALERT, "reuse connection error");
        return ngx_nrpc_close_connection(c);
    }
}

void rpc_call_core(Controller* cntl);

void Controller::finalize_client()
{
    ChannelOperateParams* params = _params;
    // in client side, also drop the connection to avoid dirty data if failed
    //   1.failed (rpc frame failed OR context error)
    //   2.success but recv eof from server
    //      release is excuted immediately after parese done if not recv eof
    if (Failed() || _client_recv_eof) {
		if (_client_sockfd != -1) {
			params->channel->connection_pool()->drop_connection(_client_sockfd);
		}
    }

    // retry only if the rpc frame failed (NOTE: not Failed())
    if (_result != RPC_OK) {
		// then retry
        if (params->max_retry_time-- > 0) {
			// TODO: some wrong,
            if (!_iobuf->read_point_resume()) {
                set_result(RPC_INNER_ERROR);
                set_result_text("can't retry (msg resume error)");
            } else {
                LOG(INFO, "channel retry [remain %d]", params->max_retry_time);
                set_client_sockfd(-1);
                set_client_recv_eof(false);
                set_result(RPC_OK);
                set_state(RPC_SESSION_SENDING);
                // still in current pthread, and finally return to rpc_call()
                return rpc_call_core(this);
            }
        } else {
			LOG(INFO, "all retries failed");
			// maybe here is a appropriate hook
			params->channel->connection_pool()->close_connection();
		}
    }

	// TODO: free resources

    return;
}

void Controller::finalize()
{
    if (!_is_server) {
        return finalize_client();
    }

    // server side
    // free iobuf
    if (_iobuf) {
        delete _iobuf;
    }

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

	// free concurrency
	free_concurrency();

    // close connection, clear timer and event OR keepalive for reuse
    finalize_server_connection(_ngx_connection);

    // free cntl, then this session is over
    // delete this;

    return;
}

}
