
/***********************************************
  File name		: rpc_session.h
  Create date	: 2015-12-02 23:47
  Modified date : 2015-12-31 03:26
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#ifndef NRPC_RPC_SESSION_H
#define NRPC_RPC_SESSION_H

extern "C" {
#include <nginx.h>
#include <ngx_core.h>
}
#include "iobuf_zero_copy_stream.h"
#include "service_set.h"
#include "server.h"
#include "protocol.h"

namespace nrpc
{

enum RPC_SESSION_STATE {
    RPC_SESSION_READING_REQUEST = 0,
    RPC_SESSION_PROCESSING,
    RPC_SESSION_SENDING_RESPONSE,
    RPC_SESSION_LOGING,
    RPC_SESSION_OVER
};

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

class RpcSession
{
public:
    RpcSession(ngx_connection_t* c);
    virtual ~RpcSession();

    bool init();

    void finalize();

    // set
    bool set_state(RPC_SESSION_STATE state);
    bool set_result(RPC_RESULT result);
    bool set_result_text(const char* result_text);
    bool set_protocol(unsigned protocol_num);
    bool set_iobuf(ngxplus::IOBuf* iobuf);

    // get
    ngxplus::IOBuf* iobuf();
    Protocol* protocol();
    void* protocol_ctx();
    RPC_SESSION_STATE state();
    ServiceSet* service_set();
    ngx_connection_t* connection();
    int32_t process_error_code();
    std::string process_error_text();

    // for server options
    int read_timeout();

private:
    RPC_SESSION_STATE _state;
    RPC_RESULT _result;
    // for inner error to describe
    const char* _result_text;

    Protocol* _protocol;
    void* _protocol_ctx;
    const char* _protocol_name;
    ServiceSet* _service_set;
    Server* _server;

    ngx_connection_t* _c;
    ngxplus::IOBuf* _iobuf;

    //RpcClientCtx _client;
    // for response meta
    int32_t _process_error_code;
    std::string _process_error_text;

};

}
#endif
