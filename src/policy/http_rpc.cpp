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

}
