#include "protocol.h"

namespace nrpc
{

class HttpProtocolCtxFactory : public ProtocolCtxFactory
{
public:
    ProtocolCtx* create_ctx() {
        return new HttpProtocolCtx();
    }
};

static HttpProtocolCtxFactory http_protocol_ctx_factory;

// Not implemented yet
struct Protocol http_protocol {
    NULL/*http_parse_message*/,
    NULL/*http_pack_request*/,
    NULL/*http_process_rpc_request*/,
    NULL/*http_process_rpc_response*/,
    &http_protocol_ctx_factory,
    "http"
};

void http_send_rpc_response(Controller* cntl)
{
    (void) cntl;
    return;
}

}
