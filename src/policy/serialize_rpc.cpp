#include <memory>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "info_log_context.h"
#include "protocol.h"
#include "rpc_session.h"
#include "controller.h"
#include "ngx_nrpc_handler.h"

namespace nrpc
{

// Interfaces of std serialize policy
static ParseResult default_parse_message(RpcSession* session, bool read_eof);
static int default_pack_request(
        ngxplus::IOBuf* msg,
        const google::protobuf::MethodDescriptor* method,
        Controller* cntl, const google::protobuf::Message& request);
static void default_process_request(RpcSession* session);
static void default_process_response(RpcSession* session);

class DefaultProtocolCtxFactory : public ProtocolCtxFactory
{
public:
    ProtocolCtx* create_ctx() {
        return new DefaultProtocolCtx();
    }
};
static DefaultProtocolCtxFactory default_protocol_ctx_factory;

struct Protocol default_protocol {
    default_parse_message,
    default_pack_request,
    default_process_request,
    default_process_response,
    &default_protocol_ctx_factory,
    "nrpc_default"
};

static bool default_nrpc_detecte_format(ngxplus::IOBuf* source, int32_t *meta_size, int32_t *body_size)
{
    if (source->get_byte_count() < 12) {
        return false;
    }
    char* rpos = source->get_read_point();
    if (*(uint32_t*)rpos != *(uint32_t*)"NRPC") {
        return false;
    }
    rpos += 4;
    *meta_size = *(int32_t*)rpos;
    rpos += 4;
    *body_size = *(int32_t*)rpos;
    return true;
}

// Parse binary format of nrpc
ParseResult default_parse_message(RpcSession* session, bool read_eof)
{
    ngxplus::IOBuf* source = session->iobuf();
    DefaultProtocolCtx* pctx = static_cast<DefaultProtocolCtx*>(session->protocol_ctx());
    if (pctx->meta_size == -1) {
        if (!default_nrpc_detecte_format(source, &pctx->meta_size, &pctx->body_size)) {
            return read_eof ? PARSE_BAD_FORMAT: PARSE_INCOMPLETE;
        }
        source->skip(12);
    }

    RpcMeta* rpc_meta = pctx->rpc_meta;
    if (!rpc_meta) {
        if (source->get_byte_count() < (size_t)pctx->meta_size) {
            return read_eof ? PARSE_BROKEN : PARSE_INCOMPLETE;
        }
        rpc_meta = new RpcMeta();
        source->cutn((int)pctx->meta_size);
        ngxplus::IOBufAsZeroCopyInputStream zero_in_stream(source);
        if (!rpc_meta->ParseFromZeroCopyStream(&zero_in_stream)) {
            session->set_result_text("parse meta error");
            return PARSE_BAD_META;
        }
        source->carrayon();
        pctx->rpc_meta = rpc_meta;
    }

    if ((int)source->get_byte_count() < pctx->body_size) {
        return read_eof ? PARSE_BROKEN : PARSE_INCOMPLETE;
    }
    return PARSE_DONE;
}

// this method will release all the remain data from msg,
// and build the default_nrpc_package with (format_describe:meta:body)
static bool default_nrpc_pack_handle(
        ngxplus::IOBuf* msg,
        const google::protobuf::Message& rpc_meta, 
        const google::protobuf::Message& rpc_body)
{
    msg->release_all();
    char* buf;
    msg->alloc(&buf, 12, ngxplus::IOBuf::IOBUF_ALLOC_EXACT);
    *(int32_t*)buf = *(int32_t*)"NRPC";
    buf += 4;

    ngxplus::IOBufAsZeroCopyOutputStream zero_out_stream(msg);
    if (!rpc_meta.SerializeToZeroCopyStream(&zero_out_stream)) {
        LOG(NGX_LOG_LEVEL_ALERT, "Failed to serialize rpc_meta in default_nrpc_pack_handle");
        return false;
    }
    int meta_size = zero_out_stream.ByteCount() - 12;
    *(int32_t*)buf = meta_size;
    buf += 4;

    if (!rpc_body.SerializeToZeroCopyStream(&zero_out_stream)) {
        LOG(NGX_LOG_LEVEL_ALERT, "Failed to serialize rpc_body(req/resp) in default_nrpc_pack_handle");
        return false;
    }
    int body_size = zero_out_stream.ByteCount() - 12 - meta_size;
    *(int32_t*)buf = body_size;

    return true;
}

// Pack `request' to `method' into `buf'.
int default_pack_request(
        ngxplus::IOBuf* msg,
        const google::protobuf::MethodDescriptor* method,
        Controller* cntl, const google::protobuf::Message& request)
{
    (void) cntl;

    RpcMeta meta;
    RpcRequestMeta* req_meta = meta.mutable_request();
    const google::protobuf::ServiceDescriptor* service = method->service();
    if (!service) {
        LOG(NGX_LOG_LEVEL_ALERT, "Failed to find the service the method_descriptor belongs to");
        return -1;
    }
    req_meta->set_service_name(service->name());
    req_meta->set_method_name(method->name());

    if (!default_nrpc_pack_handle(msg, meta, request)) {
        return -1;    
    }
    return msg->get_byte_count();
}

// Actions to a (client) request in nrpc format.
void default_process_request(RpcSession* session)
{
    ngxplus::IOBuf* req_buf = session->iobuf();
    long start_process_us = ngxplus::Timer::rawtime()/*s*/;
    const DefaultProtocolCtx* pctx = (static_cast<DefaultProtocolCtx*>(session->protocol_ctx()));
    const RpcRequestMeta& req_meta = pctx->rpc_meta->request();

    ServiceSet* service_set = static_cast<ServiceSet*>(session->service_set());
    std::string full_name = req_meta.service_name() + "_" + req_meta.method_name();
    const MethodProperty* method_property =
            service_set->find_method_property_by_full_name(full_name);
    if (!method_property) {
        session->set_result(RPC_PROCESS_ERROR);
        session->set_result_text("rpc method not found");
        return session->finalize();
    }

    const google::protobuf::MethodDescriptor* method_descriptor =
            method_property->method_descriptor;
    google::protobuf::Service* service = method_property->service;

    std::unique_ptr<google::protobuf::Message> req(
            service->GetRequestPrototype(method_descriptor).New());
    std::unique_ptr<google::protobuf::Message> resp(
            service->GetResponsePrototype(method_descriptor).New());

    // cutn() before ParseFromZeroCopyStream
    int body_size = pctx->body_size;
    req_buf->cutn(body_size);
    ngxplus::IOBufAsZeroCopyInputStream zero_in_stream(req_buf);
    if (!req->ParseFromZeroCopyStream(&zero_in_stream)) {
        session->set_result(RPC_PROCESS_ERROR);
        session->set_result_text("Failed to parse rpc request");
        return session->finalize();
    }
    // release the remian payload
    req_buf->carrayon();
    req_buf->release_all();

    std::unique_ptr<Controller> cntl(new (std::nothrow) Controller);
    cntl->set_process_start_time(start_process_us);
    // client.options --> req_meta -->server.cntl
    cntl->set_session(session);
    cntl->set_request(req.get());
    cntl->set_response(resp.get());

    google::protobuf::Closure* done = google::protobuf::NewCallback<Controller*>(
            &default_send_rpc_response, cntl.get());
 
    return service->CallMethod(method_descriptor, cntl.release(),
            req.release(), resp.release(), done);
}

// Actions to a (server) response in nrpc format.
// for client end
void default_process_response(RpcSession* session)
{
    (void) session;
    ngxplus::IOBuf* resp_buf = session->iobuf();
    (void) resp_buf;

    return;
}

void default_send_rpc_response(Controller* cntl)
{
    long start_process_us = cntl->process_start_time();
    (void) start_process_us;
    RpcSession* rpc_session = cntl->session();
    const google::protobuf::Message* resp = cntl->response();

    RpcMeta* rpc_meta = ((DefaultProtocolCtx*)rpc_session->protocol_ctx())->rpc_meta;
    RpcResponseMeta* response_meta = rpc_meta->mutable_response();
    response_meta->set_error_code(rpc_session->process_error_code());
    response_meta->set_error_text(rpc_session->process_error_text());
    ngxplus::IOBuf* iobuf = rpc_session->iobuf();

    // pack nrpc default packages
    if (!default_nrpc_pack_handle(iobuf, *rpc_meta, *resp)) {
        rpc_session->set_result(RPC_INNER_ERROR);
        rpc_session->set_result_text("serialize response pack error");
        rpc_session->finalize();
    }

    // really send response data to client
    rpc_session->set_state(RPC_SESSION_SENDING_RESPONSE);
    ngx_connection_t* c = rpc_session->connection();
    c->write->handler = ngx_nrpc_send_response;
    return ngx_nrpc_send_response(c->write);
}

}
