extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
}
#include <iostream>

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

bool Channel::connect_with_timeout(int socket, struct sockaddr* addr, socklen_t addr_len, int timeout)
{
    timeval timeval = {timeout, 0};

    // set the socket in non-blocking
    int flags = fcntl(socket, F_GETFL);
    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK)) {
        LOG(NGX_LOG_LEVEL_ALERT, "fcntl failed with error: %d\n", errno);
        return false;
    }

    if (connect(socket, addr, addr_len) == 0) {
        return true;
    }
    if (errno != EINPROGRESS) {
        LOG(NGX_LOG_LEVEL_ALERT, "connect error: %d\n", errno);
        return false;
    }

    // reset the flags
    flags = fcntl(socket, F_GETFL);
    if (fcntl(socket, F_SETFL, flags & ~O_NONBLOCK)) {
        LOG(NGX_LOG_LEVEL_ALERT, "fcntl failed with error: %d\n", errno);
        return false;
    }

    fd_set write_set, err_set;
    FD_ZERO(&write_set);
    FD_ZERO(&err_set);
    FD_SET(socket, &write_set);
    FD_SET(socket, &err_set);

    // check if the socket is ready, select will block timeval
    select(0, NULL, &write_set, &err_set, &timeval);
    if(FD_ISSET(socket, &write_set))
        return true;

    return false;
}

bool Channel::init(const char* server_addr, int port, const ChannelOption* option)
{
    _server_ip = server_addr;
    _server_port = port;
    bzero(&_servaddr, sizeof(_servaddr));
    _servaddr.sin_family = AF_INET;
    _servaddr.sin_port = htons(_server_port);
    int ret = inet_pton(AF_INET, _server_ip, &_servaddr.sin_addr);
    if (ret <= 0) {
        LOG(NGX_LOG_LEVEL_ALERT, "error ip format \"%s\"", _server_ip);
        return false;
    }
    _servaddr_len = sizeof(_servaddr);
 
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
    //cntl->set_request(request);
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
        cntl->set_result_text("create sockfd erro");
        return cntl->finalize();
    }
    if (!connect_with_timeout(sockfd, (struct sockaddr*)&_servaddr, _servaddr_len,
                _option->connection_timeout)) {
        cntl->set_result(RPC_SEND_ERROR);
        cntl->set_result_text("connect server erro");
        return cntl->finalize();
    }

    // implement a simply sync mode client
    // here set the timeout for send and recv
    struct timeval send_timeout={_option->send_timeout, 0};
    struct timeval read_timeout={_option->read_timeout, 0};
    int ret=setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&send_timeout,sizeof(read_timeout));
    ret=setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&read_timeout,sizeof(read_timeout));

    send(sockfd, msg->get_read_point(), msg->get_byte_count(), 0);
    // TODO:process errno, again or finalize if failed

    msg->release_all();
    // just implement a sync mode client
    ngxplus::IOBufAsZeroCopyOutputStream zero_out_stream(msg);
    char *buf;
    int size;
    while (zero_out_stream.Next((void**)&buf, &size)) {
        if (size == 0) {
            continue;
        }
        int n = recv(sockfd, buf, size, 0);
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

    default_protocol.process_response(cntl);
}

}
