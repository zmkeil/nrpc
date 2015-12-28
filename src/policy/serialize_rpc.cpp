#include <memory>
#include "info_log_context.h"
#include "protocol.h"

namespace nrpc
{

// Interfaces of std serialize policy
static ParseResult default_parse_message(ngxplus::IOBuf* source, RpcSession* session, bool read_eof);
static int default_pack_request(
        ngxplus::IOBuf* msg,
        const google::protobuf::MethodDescriptor* method,
        Controller* cntl,
        const google::protobuf::Message& request);
static void default_process_request(RpcSession* session, RpcMeta& req_meta, ngxplus::IOBuf* req_buf);
static void default_process_response(RpcSession* session, RpcMeta& resp_meta, ngxplus::IOBuf* resp_buf);

struct Protocol default_protocol {
    default_parse_message,
    default_pack_request,
    default_process_request,
    default_process_response,
    "nrpc_default"
};

// Parse binary format of baidu-rpc
ParseResult default_parse_message(ngxplus::IOBuf* source, RpcSession* session, bool read_eof)
{
    (void) source;
    (void) session;
    (void) read_eof;
    return PARSE_DONE;
}

// Pack `request' to `method' into `buf'.
int default_pack_request(
        ngxplus::IOBuf* msg,
        const google::protobuf::MethodDescriptor* method,
        Controller* cntl,
        const google::protobuf::Message& request)
{
    (void) cntl;
    RpcMeta meta;
    RpcRequestMeta* req_meta = meta.mutable_request();
    const google::protobuf::ServiceDescriptor* service = method->service();
    if (!service) {
        return -1;
    }
    req_meta->set_service_name(service->name());
    req_meta->set_method_name(method->name());

    ngxplus::IOBufAsZeroCopyOutputStream zero_out_stream(msg);
    if (!meta.SerializeToZeroCopyStream(&zero_out_stream)) {
        return -1;
    }
    return request.SerializeToZeroCopyStream(&zero_out_stream) ?
            zero_out_stream.ByteCount() : -1;
}

// Actions to a (client) request in baidu-rpc format.
void default_process_request(RpcSession* session, RpcMeta& meta, ngxplus::IOBuf* req_buf)
{
    const RpcRequestMeta& req_meta = meta.request();

    ServiceSet* service_set = static_cast<ServiceSet*>(session->service_set);
    std::string full_name = req_meta.service_name() + "_" + req_meta.method_name();
    const MethodProperty* method_property =
            service_set->find_method_property_by_full_name(full_name);
    if (!method_property) {
        LOG(NGX_LOG_LEVEL_ALERT, "rpc method not found");
        // session->set_error()
        return;
    }

    const google::protobuf::MethodDescriptor* method_descriptor =
            method_property->method_descriptor;
    google::protobuf::Service* service = method_property->service;


    std::unique_ptr<google::protobuf::Message> req(
            service->GetRequestPrototype(method_descriptor).New());
    std::unique_ptr<google::protobuf::Message> res(
            service->GetResponsePrototype(method_descriptor).New());
    ngxplus::IOBufAsZeroCopyOutputStream zero_out_stream(req_buf);
    if (!req->SerializeToZeroCopyStream(&zero_out_stream)) {
        LOG(NGX_LOG_LEVEL_ALERT, "rpc request not fitted");
        // session->set_error()
        return;
    }

    std::unique_ptr<Controller> cntl(new (std::nothrow) Controller);
    // client.options --> req_meta -->server.cntl

/*     google::protobuf::Closure* done = google::protobuf::NewCallback<
 *         int64_t, Controller*, const google::protobuf::Message*,
 *         const google::protobuf::Message*, Socket*, const Server*,
 *         MethodStatus*, long>(
 *             &SendRpcResponse, meta.correlation_id(), cntl.get(),
 *             req.get(), res.get(), socket.release(), server.release(),
 *             sp->status, start_parse_us);
 */
    return service->CallMethod(method_descriptor, cntl.release(),
            req.release(), res.release(), NULL/*done*/);
}

// Actions to a (server) response in baidu-rpc format.
void default_process_response(RpcSession* session, RpcMeta& resp_meta, ngxplus::IOBuf* resp_buf)
{
    (void) session;
    (void) resp_meta;
    (void) resp_buf;

    return;
}

}
