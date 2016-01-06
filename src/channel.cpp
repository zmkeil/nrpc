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

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("create sockfd error");
        return cntl->finalize();
    }

    if (!common::connect_with_timeout(sockfd, (struct sockaddr*)&_servaddr, _servaddr_len, _option->connection_timeout)) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("connect error");
        cntl->finalize();
    }

    int size = common::send_with_timeout(sockfd, msg->get_read_point(), msg->get_byte_count(), _option->send_timeout);
    if (size < (int)msg->get_byte_count()) {
        // TODO:process errno, again or finalize if failed
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("send error");
        cntl->finalize();
    }

    msg->release_all();
    cntl->set_state(RPC_SESSION_READING);
    // just implement a sync mode client
    ngxplus::IOBufAsZeroCopyOutputStream zero_out_stream(msg);
    char *buf;
    while (zero_out_stream.Next((void**)&buf, &size)) {
        if (size == 0) {
            continue;
        }
        int n = common::recv_with_timeout(sockfd, buf, size, _option->read_timeout);
        if (n < 0) {
            cntl->set_result(RPC_READ_ERROR);
            cntl->set_result_text("read error or timeout");
            return cntl->finalize();
        }
        if ((n == 0) || (n < size)) {
            // parse, then determine process_response or next loop
            // here just process_response
            break;
        }
    }
    // parse in read process
    default_protocol.parse(cntl, true);
    cntl->set_state(RPC_SESSION_PROCESSING);

    return default_protocol.process_response(cntl);
}

}
