#include "protocol.h"

namespace nrpc
{

// Not implemented yet
struct Protocol http_protocol {
    NULL/*http_parse_message*/,
    NULL/*http_pack_request*/,
    NULL/*http_process_rpc_request*/,
    NULL/*http_process_rpc_response*/,
    "http"
};

void http_send_rpc_response(Controller* cntl, const google::protobuf::Message* req,
                const google::protobuf::Message* resp, long start_process_us)
{
    (void) cntl;
    (void) req;
    (void) resp;
    (void) start_process_us;
    return;
}

}
