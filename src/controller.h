
/***********************************************
  File name		: controller.h
  Create date	: 2015-12-02 23:47
  Modified date : 2016-01-19 17:23
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#ifndef NRPC_CONTROLLER_H
#define NRPC_CONTROLLER_H

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <io/iobuf_zero_copy_stream.h>
#include <ngxplus_iobuf.h>
#include "service_set.h"
#include "server.h"
#include "protocol.h"
#include "service_context.h"

struct ngx_connection_s;
typedef struct ngx_connection_s ngx_connection_t;

namespace nrpc
{

struct ChannelOperateParams;

enum RPC_SESSION_STATE {
    RPC_SESSION_READING = 0,
    RPC_SESSION_PROCESSING,
    RPC_SESSION_SENDING,
    RPC_SESSION_LOGING,
    RPC_SESSION_OVER
};
/* In client side, the order is:
 *     RPC_SESSION_SENDING
 *     RPC_SESSION_READING
 *     RPC_SESSION_PROCESSING
 *     RPC_SESSION_LOGING
 *     RPC_SESSION_OVER
 */

// rpc frame result
enum RPC_RESULT {
    // inner error, just close connection
    RPC_INNER_ERROR = 0,
    // first two result, just close connection
    // don't send response
    RPC_READ_ERROR,
    RPC_READ_TIMEOUT,
    // set response_meta's error code/text
    RPC_PROCESS_ERROR,
    RPC_PROCESS_TIMEOUT,
    // close connection
    RPC_SEND_ERROR,
    RPC_SEND_TIMEOUT,
    // send all response_payload to TCP_BUFFER,
    // cann't ensure client receive it correctly,
    // in server end, this RPC is ok
    RPC_OK
};

class Controller : public google::protobuf::RpcController
{
public:
    Controller();
    virtual ~Controller();

    // -------------------------------------------------------------------
    //                      Client-side methods
    // These calls may be made from the client side only.  Their results
    // are undefined on the server side (may crash).
    // -------------------------------------------------------------------

    bool Failed() const;

    std::string ErrorText() const;

	void set_channel_operate_params(ChannelOperateParams* params)
	{
		_params = params;
	}
	ChannelOperateParams* channel_operate_params()
	{
		return _params;
	}
	void set_client_sockfd(int sockfd)
	{
		_client_sockfd = sockfd;
	}
	int client_sockfd()
	{
		return _client_sockfd;
	}
	void set_client_recv_eof(bool recv_eof)
	{
		_client_recv_eof = recv_eof;
	}
	bool recv_eof()
	{
		return _client_recv_eof;
	}

    // TODO: reuse controller
    virtual void Reset() {
        return;
    }

    // TODO: in asyn mode, start a rpc-call, an then cancel it before the response back
    virtual void StartCancel() {
        return;
    }


    // -------------------------------------------------------------------
    //                      Server-side methods
    // These calls may be made from the server side only.  Their results
    // are undefined on the client side (may crash).
    // -------------------------------------------------------------------

    // In user_defined service->method, use this api to set the reson into rpc_meta's
    // error_code and error_text
    void SetFailed(const std::string& reason);

    // TODO: can't implement
    virtual bool IsCanceled() const {
        return true;
    }

    // TODO: can't implement
    virtual void NotifyOnCancel(google::protobuf::Closure* callback) {
        (void) callback;
        return;
    }

    // server side init, get _server, _service_set from c
    bool server_side_init(ngx_connection_t* c);

    // service_set and ngx_connection
    bool set_service_set(ServiceSet* service_set) {
        _service_set = service_set;
        return true;
    }
    ServiceSet* service_set() {
        return _service_set;
    }
    ngx_connection_t* connection() {
        return _ngx_connection;
    }

    // for server options
    ServiceContext* service_context();
    int server_read_timeout();
    int server_write_timeout();
	bool get_concurrency();
	void free_concurrency();


    // -------------------------------------------------------------------
    //                      Both-side methods.
    // Following methods can be called from both client and server. But they
    // may have different or opposite semantics.
    // -------------------------------------------------------------------
    // session state and result
    bool set_state(RPC_SESSION_STATE state) {
        _state = state;
        return true;
    }
    bool set_result(RPC_RESULT result) {
        _result = result;
        return true;
    }
    bool set_result_text(const char* result_text) {
        _result_text = result_text;
        return true;
    }
    RPC_SESSION_STATE state() {
        return _state;
    }

    // protocol
    bool set_protocol(unsigned protocol_num);

    Protocol* protocol() {
        return _protocol;
    }
    void* protocol_ctx() {
        return _protocol_ctx;
    }
 
    // rpc data
    void set_request(google::protobuf::Message* request) {
        _request = request;
    }
    google::protobuf::Message* request() {
        return _request;
    }
    void set_response(google::protobuf::Message* response) {
        _response = response;
    }
    google::protobuf::Message* response() {
        return _response;
    }


    // iobuf of the rpc procedure
    bool set_iobuf(ngxplus::NgxplusIOBuf* iobuf) {
        _iobuf = iobuf;
        return true;
    }
    ngxplus::NgxplusIOBuf* iobuf() {
        return _iobuf;
    }

    // stastics
    int/*base::EndPoint*/ remote_side() const {
        return _remote_side;
    }
    int/*base::EndPoint*/ local_side() const {
        return _local_side;
    }
    void set_start_time_s(long start_time_s) {
        _start_time_s = start_time_s;
    }
    long start_time_s() {
        return _start_time_s;
    }
    void set_start_time_us(long start_time_us) {
        _start_time_us = start_time_us;
    }
    long start_time_us() {
        return _start_time_us;
    }
    void set_start_process_time_us(long start_process_time_us) {
        _start_process_time_us = start_process_time_us;
    }
    long start_process_time_us() {
        return _start_process_time_us;
    }
    void set_end_process_time_us(long end_process_time_us) {
        _end_process_time_us = end_process_time_us;
    }
    long end_process_time_us() {
        return _end_process_time_us;
    }
    void set_end_time_us(long end_time_us) {
        _end_time_us = end_time_us;
    }
    long end_time_us() {
        return _end_time_us;
    }

    // finalize
    void finalize();

private:
    void finalize_server_connection(ngx_connection_t* c);
    void finalize_client();

private:
    // _is_server
    bool _is_server;

    // rpc_frame describes for both of server and client side;
    // the order of _state is different between server and client side;
    // the _result and _result_text describe the rpc_frame errors,
    // the user_define rpc_service errors are transmited from server to client
    // by rpc_meta->response
    RPC_SESSION_STATE _state;
    RPC_RESULT _result;
    const char* _result_text;

    // protocol for both of server and client side
    Protocol* _protocol;
    void* _protocol_ctx;
    const char* _protocol_name;

    // rpc data for both of server and client side
    // In server: set req and resp in protocol.process_request, then excute user_defined
    // method, and then get resp in default_send_rpc_response and send it to client.
    // In client: set resp in stud->method(Channel->CallMethod), then get in protocol.
    // process_response
    google::protobuf::Message* _request;
    google::protobuf::Message* _response;

    // iobuf for both of server and client side
    // In server: first >> iobuf and then << iobuf, In client: excuted in contrast
    // destory it in finalize()
    ngxplus::NgxplusIOBuf* _iobuf;

    // stastics for both of server and client side
    long _start_time_s;
    long _start_time_us;
    long _start_process_time_us;
    long _end_process_time_us;
    long _end_time_us;
    int _remote_side;
    int _local_side;


    // server and service informations for server side
    ServiceSet* _service_set;
    Server* _server;
    ServiceContext* _service_context;
    ngx_connection_t* _ngx_connection;

    // client info
    // the current rpc_call ctx, used in retry
    ChannelOperateParams* _params;
	int _client_sockfd;
	bool _client_recv_eof;
};

}
#endif
