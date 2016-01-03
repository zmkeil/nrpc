
/***********************************************
  File name		: controller.h
  Create date	: 2015-12-28 20:50
  Modified date	: 2015-12-28 20:50
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include <google/protobuf/service.h>
#include "rpc_session.h"
#include "protocol.h"

namespace nrpc
{

class Controller : public google::protobuf::RpcController
{
public:
    Controller() {}
    virtual  ~Controller() {};
    // ------------------------------------------------------------------
    //                      Client-side methods
    // These calls shall be made from the client side only.  Their results
    // are undefined on the server side (may crash).
    // ------------------------------------------------------------------)
    // timeout, max_retry, log_id e.g.


    // ------------------------------------------------------------------------
    //                      Server-side methods.
    // These calls shall be made from the server side only. Their results are
    // undefined on the client side (may crash).
    // ------------------------------------------------------------------------

    // If true, indicates that the client canceled the RPC or the connection has
    // broken, so the server may as well give up on replying to it.  The server
    // should still call the final "done" callback.
    bool IsCanceled() const {return true;}

    // Asks that the given callback be called when the RPC is canceled or the
    // connection has broken.  The callback will always be called exactly once.
    // If the RPC completes without being canceled/broken connection, the callback
    // will be called after completion.  If the RPC has already been canceled/broken
    // when NotifyOnCancel() is called, the callback will be called immediately.
    //
    // NotifyOnCancel() must be called no more than once per request.
    void NotifyOnCancel(google::protobuf::Closure* callback) {
        (void) callback;
    }

    // Return the protocol of this request
    // ProtocolType request_protocol() const { return _request_protocol; }
    Protocol* request_protocol() const {
        return _session->protocol();
    }

    // Tell RPC to close the connection instead of sending back response.
    // If this controller was not SetFailed() before, ErrorCode() will be
    // set to ECLOSE.
    // Notice that the actual closing does not take place immediately.
    void CloseConnection(const char* reason_fmt, ...) {
        (void) reason_fmt;
    }

    // True if CloseConnection() was called.
    bool IsCloseConnection() const { return _is_close_connection; }

    // The server running this RPC session.
    // Always NULL at client-side.
    const Server* server() const { return _server; }

    // Get the data attached to current RPC session. The data is created by 
    // ServerOptions.session_local_data_factory and reused between different
    // RPC. If factory is NULL, this method returns NULL.
    void* session_local_data() {return NULL;}


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

    // Resets the Controller to its initial state so that it may be reused in
    // a new call.  Must NOT be called while an RPC is in progress.
    void Reset() { /*InternalReset(false);*/ }

    // Causes Failed() to return true on the client side.  "reason" will be
    // incorporated into the message returned by ErrorText().
    void SetFailed(const std::string& reason) {
        (void) reason;
    }
    void SetFailed(int error_code, const char* reason_fmt, ...) {
        (void) error_code;
        (void) reason_fmt;
    }

    // After a call has finished, returns true if the RPC call failed.
    // The response to Channel is undefined when Failed() is true.
    // Calling Failed() before a call has finished is undefined.
    bool Failed() const {return false;}

    // If Failed() is true, return description of the errors.
    // NOTE: ErrorText() != berror(ErrorCode()). 
    std::string ErrorText() const {return std::string("error");}

    // Last error code. Equals 0 iff Failed() is false.
    // If there's retry, latter code overwrites former one.
    int ErrorCode() const { return _error_code; }

    void StartCancel() {return;}

    
    void set_session(RpcSession* session) {_session = session;}
    RpcSession* session() {return _session;}
    void set_request(google::protobuf::Message* request) {_request = request;}
    google::protobuf::Message* request() {return _request;}
    void set_response(google::protobuf::Message* response) {_response = response;}
    google::protobuf::Message* response() {return _response;}

    void set_process_start_time(long start_process_us) {_start_process_us = start_process_us;}
    long process_start_time() {return _start_process_us;}

private:
    bool _is_close_connection;
    Server* _server;
    int _remote_side;
    int _local_side;
    int _error_code;

    RpcSession* _session;
    google::protobuf::Message* _request;
    google::protobuf::Message* _response;

    long _start_process_us;
};

}
