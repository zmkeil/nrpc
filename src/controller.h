
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
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
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

class Controller : public google::protobuf::RpcController
{
public:
    Controller(ngx_connection_t* c);
    virtual ~Controller();

    // Client-side methods ---------------------------------------------
    // These calls may be made from the client side only.  Their results
    // are undefined on the server side (may crash).

    // Resets the RpcController to its initial state so that it may be reused in
    // a new call.  Must not be called while an RPC is in progress.
    virtual void Reset() {return;}

    // After a call has finished, returns true if the call failed.  The possible
    // reasons for failure depend on the RPC implementation.  Failed() must not
    // be called before a call has finished.  If Failed() returns true, the
    // contents of the response message are undefined.
    virtual bool Failed() const {return true;}

    // If Failed() is true, returns a human-readable description of the error.
    virtual std::string ErrorText() const {return "OK";}

    // Advises the RPC system that the caller desires that the RPC call be
    // canceled.  The RPC system may cancel it immediately, may wait awhile and
    // then cancel it, or may not even cancel the call at all.  If the call is
    // canceled, the "done" callback will still be called and the RpcController
    // will indicate that the call failed at that time.
    virtual void StartCancel() {return;}


    // Server-side methods ---------------------------------------------
    // These calls may be made from the server side only.  Their results
    // are undefined on the client side (may crash).

    // Causes Failed() to return true on the client side.  "reason" will be
    // incorporated into the message returned by ErrorText().  If you find
    // you need to return machine-readable information about failures, you
    // should incorporate it into your response protocol buffer and should
    // NOT call SetFailed().
    virtual void SetFailed(const std::string& reason) {return;}

    // If true, indicates that the client canceled the RPC, so the server may
    // as well give up on replying to it.  The server should still call the
    // final "done" callback.
    virtual bool IsCanceled() const {return true;}

    // Asks that the given callback be called when the RPC is canceled.  The
    // callback will always be called exactly once.  If the RPC completes without
    // being canceled, the callback will be called after completion.  If the RPC
    // has already been canceled when NotifyOnCancel() is called, the callback
    // will be called immediately.
    //
    // NotifyOnCancel() must be called no more than once per request.
    virtual void NotifyOnCancel(google::protobuf::Closure* callback) {return;}

    // -------------------------------------------------------------------
    //                      Both-side methods.
    // Following methods can be called from both client and server. But they
    // may have different or opposite semantics.
    // -------------------------------------------------------------------

    // Client-side: client-side remote_side() is surprisingly tricky. 
    // Before each retry, it is the server that request will be sent to 
    // and protocols can fetch this information in PackXXXRequest functions. 
    // When RPC succeeds, it is the server which successfully responded. If
    // the RPC failed, it's the last server tried.
    // Server-side: returns the client sending the request
    int/*base::EndPoint*/ remote_side() const { return _remote_side; }

    // Client-side: client-side local_side() is undefined until this RPC
    // succeeds
    // Server-side: returns the address that client sends to
    int/*base::EndPoint*/ local_side() const { return _local_side; }



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

    // add from cntl
    void set_process_start_time(long start_process_us) {_start_process_us = start_process_us;}
    long process_start_time() {return _start_process_us;}

    void set_request(google::protobuf::Message* request) {_request = request;}
    google::protobuf::Message* request() {return _request;}
    void set_response(google::protobuf::Message* response) {_response = response;}
    google::protobuf::Message* response() {return _response;}


private:
    bool set_service_set(ServiceSet* service_set);

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


    // add from cntl
    int _remote_side;
    int _local_side;
    int _error_code;

    google::protobuf::Message* _request;
    google::protobuf::Message* _response;

    long _start_process_us;

};

}
#endif
