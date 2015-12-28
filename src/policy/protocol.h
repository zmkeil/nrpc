#ifndef NRPC_PROTOCOL_H
#define NRPC_PROTOCOL_H

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "iobuf_zero_copy_stream.h"
#include "rpc_session.h"
#include "service_set.h"
#include "server.h"
#include "controller.h"
#include "proto/nrpc_meta.pb.h"

namespace nrpc
{

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

struct Protocol {
    Parse parse;

    PackRequest pack_request;

    ProcessRequest process_request;

    ProcessResponse process_response;

    const char* name;
};

extern struct Protocol default_protocol;
extern struct Protocol http_protocol;

}
#endif
