#ifndef NRPC_PROTOCOL_H
#define NRPC_PROTOCOL_H

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "iobuf_zero_copy_stream.h"
#include "service_set.h"
#include "server.h"
#include "proto/nrpc_meta.pb.h"

namespace nrpc
{

class RpcSession;
class Controller;

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
typedef ParseResult (*Parse)(ngxplus::IOBuf* source, RpcSession* session, bool read_eof);

typedef int (*PackRequest)(
        ngxplus::IOBuf* msg,
        const google::protobuf::MethodDescriptor* method,
        Controller* controller,
        const google::protobuf::Message& request);

typedef void (*ProcessRequest)(RpcSession* session, RpcMeta& req_meta, ngxplus::IOBuf* req_buf);

typedef void (*ProcessResponse)(RpcSession* session, RpcMeta& resp_meta, ngxplus::IOBuf* resp_buf);

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
    RpcMeta* rpc_meta;
    // for request
    int32_t meta_size;
    int32_t body_size;

    // for response
};
void default_send_rpc_response(Controller* cntl, long start_process_us);

struct HttpProtocolCtx : public ProtocolCtx {
    int version;
};
void http_send_rpc_response(Controller* cntl, long start_process_us);

}
#endif
