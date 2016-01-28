#ifndef NRPC_PROTOCOL_H
#define NRPC_PROTOCOL_H

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <ngxplus_iobuf.h>
#include "service_set.h"
#include "server.h"

namespace nrpc
{

class Controller;
class RpcMeta;

enum ParseResult {
    // bad package format
    PARSE_BAD_FORMAT = 0,
    // socket receive continue
    PARSE_INCOMPLETE,
    // receive read_eof, but still INCOMPLETE
    PARSE_BROKEN,
    // bak meta
    PARSE_BAD_META,
    // done
    PARSE_DONE
};

// A set interface of a protocol
typedef ParseResult (*Parse)(Controller* cntl, bool read_eof);

typedef int (*PackRequest)(
        ngxplus::NgxplusIOBuf* msg,
        const google::protobuf::MethodDescriptor* method,
        Controller* cntl, const google::protobuf::Message& request);

typedef void (*ProcessRequest)(Controller* cntl);

typedef void (*ProcessResponse)(Controller* cntl);

class ProtocolCtx {
};

class ProtocolCtxFactory {
public:
    virtual ProtocolCtx* create_ctx() = 0;
    virtual void destory_ctx(ProtocolCtx* ctx) {
        delete ctx;
    }
};

struct Protocol {
    Parse parse;

    PackRequest pack_request;

    ProcessRequest process_request;

    ProcessResponse process_response;

    ProtocolCtxFactory* ctx_factory;

    const char* name;
};



/*
 * for all specially protocols
 * declare g_rpc_protocols, and define all protocol's ctx
 * and declare all protocol's send_response_result
 */

#define NRPC_MAX_PROTOCOL_NUM 10
#define NRPC_PROTOCOL_DEFAULT_NUM 0
#define NRPC_PROTOCOL_HTTP_NUM 1

extern Protocol default_protocol;
extern Protocol http_protocol;
extern Protocol* g_rpc_protocols[NRPC_MAX_PROTOCOL_NUM];

struct DefaultProtocolCtx : public ProtocolCtx {
    DefaultProtocolCtx() : rpc_meta(nullptr), meta_size(-1), body_size(-1) {}
    RpcMeta* rpc_meta;
    // for request
    // total_size = 12 + meta_size + body_size
    int32_t meta_size;
    int32_t body_size;

    // for response
};
void default_send_rpc_response(Controller* cntl, bool real_send);
void default_send_rpc_response(Controller* cntl);

struct HttpProtocolCtx : public ProtocolCtx {
    int version;
};
void http_send_rpc_response(Controller* cntl);

}
#endif
