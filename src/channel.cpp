extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
}
#include <iostream>
#include <common.h>
#include "info_log_context.h"
#include "controller.h"
#include "protocol.h"

#include "channel.h"

namespace nrpc {

Channel::Channel()
{
}

Channel::~Channel()
{
}

bool Channel::init(const char* server_addr, int port, const ChannelOption* option)
{
    _server_ip = server_addr;
    _server_port = port;
    _servaddr_len = sizeof(_servaddr);
    if (!common::sockaddr_init(&_servaddr, _servaddr_len, server_addr, port)) {
        LOG(NGX_LOG_LEVEL_ALERT, "error ip format \"%s\"", _server_ip);
        return false;
    }

    _option = option;
    return true; 
}

struct ChannelOperateParams {
    ChannelOperateParams(Channel* channel, Controller* cntl, google::protobuf::Closure* done) :
            channel(channel), cntl(cntl), done(done) {}
    Channel* channel;
    Controller* cntl;
    google::protobuf::Closure* done;
};

static void rpc_call_core(ChannelOperateParams* params)
{
    Channel* channel = params->channel;
    Controller* cntl = params->cntl;
    ngxplus::IOBuf* msg = cntl->iobuf();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("create sockfd error");
        return cntl->finalize();
    }

    if (!common::connect_with_timeout(sockfd, channel->servaddr(),
                channel->servaddr_len(), channel->option()->connection_timeout)) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("connect error");
        return cntl->finalize();
    }

    char *send_buf;
    char *recv_buf;
    int size;
    int remain_len;
    ngxplus::IOBufAsZeroCopyInputStream zero_in_stream(msg);
    while (zero_in_stream.Next((const void**)&send_buf, &size)) {
        if (size == 0) {
            continue;
        }
        remain_len = size;
        while (remain_len > 0) {
            int current_send_len = common::send_with_timeout(sockfd, send_buf,
                    remain_len, channel->option()->send_timeout);
            if (current_send_len < 0) {
                cntl->set_result(RPC_SEND_ERROR);
                cntl->set_result_text("send error or timeout");
                return cntl->finalize();
            }
            remain_len -= current_send_len;
            send_buf += current_send_len;
        }
    }

    msg->release_all();
    cntl->set_state(RPC_SESSION_READING);

    bool recv_eof = false;
    ngxplus::IOBufAsZeroCopyOutputStream zero_out_stream(msg);
    while (zero_out_stream.Next((void**)&recv_buf, &size)) {
        if (size == 0) {
            continue;
        }
        remain_len = size;
        while (remain_len > 0) {
            int current_recv_len = common::recv_with_timeout(sockfd, recv_buf,
                    remain_len, channel->option()->read_timeout);
            if (current_recv_len < 0) {
                cntl->set_result(RPC_READ_ERROR);
                cntl->set_result_text("read error or timeout");
                return cntl->finalize();
            }
            if (current_recv_len == 0) {
                recv_eof = true;
            }
            ParseResult parse_result =  default_protocol.parse(cntl, recv_eof);
            if (parse_result == PARSE_DONE) {
                // parse in read process
                cntl->set_state(RPC_SESSION_PROCESSING);
                return default_protocol.process_response(cntl);
            } else if (parse_result == PARSE_INCOMPLETE) {
                remain_len -= current_recv_len;
                recv_buf += current_recv_len;
            } else {
                // error
                cntl->set_result(RPC_INNER_ERROR);
                cntl->set_result_text("parse response error");
                return cntl->finalize();
            }
        }
    }
}

static void* rpc_call(void* arg)
{
/*     make_scope([] () {
 *             delete params;
 *         }
 *     );
 */
    ChannelOperateParams* params = static_cast<ChannelOperateParams*>(arg);
    google::protobuf::Closure* done = params->done;
    // process channel state, reuse, joinable calls

    rpc_call_core(params);
    if (done) {
        done->Run();
    }
    return NULL;
}

void Channel::CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done)
{
    // async mode not implemented yet
    (void) done;
    Controller* cntl = static_cast<Controller*>(controller);
    cntl->set_state(RPC_SESSION_SENDING);
    cntl->set_response(response);
    cntl->set_protocol(NRPC_PROTOCOL_DEFAULT_NUM);

    ngxplus::IOBuf* msg = new ngxplus::IOBuf();
    int bytes = default_protocol.pack_request(msg, method, cntl, *request);
    if (bytes == -1) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("pack error");
        return cntl->finalize();
    }
    cntl->set_iobuf(msg);

    // so channel must be deconstructed after rpc_call
    // In sync mode, this usually is not problem, In async mode, be carefully
    ChannelOperateParams* channel_operate_params = new ChannelOperateParams(this, cntl, done);
    pthread_t thid;
    if (pthread_create(&thid, NULL/*joinable*/, &rpc_call, channel_operate_params) != 0) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("can't create pthread");
        return cntl->finalize();
    }
    if (!done) {
        pthread_join(thid, NULL);
        return;
    }
    else {
        // not implemented yet
        return;
    }
}

bool Channel::channel_join()
{
    pthread_join(_join_thread_ids, NULL);
    return true;
}

}
