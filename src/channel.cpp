extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
}
#include <iostream>
#include "rpc_session.h"
#include "protocol.h"

#include "channel.h"

namespace nrpc {

NrpcChannel::NrpcChannel(const std::string& server_address)
{
    // check address availability, give the ip:port

}

NrpcChannel::~NrpcChannel()
{
}

void NrpcChannel::CallMethod(const MethodDescriptor* method,
        RpcController* controller, const Message* request,
        Message* response, Closure* done)
{
    // async mode not implemented yet
    (void) done;
    NrpcController* cntl = static_cast<RpcController*>(controller);

    ngxplus::IOBuf* msg = new ngxplus::IOBuf();
    int bytes = default_protocol.pack_request(msg, method, cntl, *request);
    if (bytes == -1) {
        cntl->set_error_code(NRPC_CLIENT_PACK_ERROR);
        cntl->set_error_text("pack error");
        return;
    }

    RpcSession* session = new RpcSession();
    session->set_cntl(cntl);
    session->set_iobuf(msg);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(_server_port);
    inet_pton(AF_INET, _server_ip, &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    send(sockfd, msg.get_read_point(), msg->get_byte_count(), 0);

    msg.release_all();
    // just implement a sync mode client
    ngxplus::IOBufAsZeroCopyOutputStream zero_out_stream(&iobuf);
    char *buf;
    int size;
    while (zero_out_stream.Next((void**)&buf, &size)) {
        if (size == 0) {
            continue;
        }
        int n = recv(sockfd, buf, size, 0);
        std::cout << n << std::endl;
        if (n < 0) {
            std::cout << "read error" << std::endl;
            return -1;
        }
        if ((n == 0) || (n < size)) {
            // parse, then determine process_response or next loop
            // here just process_response
            break;
        }
    }
    // parse in read process
    default_protocol.parse(session, true);

    default_protocol.process_response(session);
}

}
