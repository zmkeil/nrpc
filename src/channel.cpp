extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
}
#include <iostream>
#include <algorithm>
#include <common.h>
#include "info_log_context.h"
#include "controller.h"
#include "protocol.h"
#include "channel.h"

namespace nrpc {

Channel::Channel()
{
    _mutex = PTHREAD_MUTEX_INITIALIZER;
}

Channel::~Channel()
{
    pthread_mutex_destroy(&_mutex);
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

int Channel::new_connection()
{
    LOG(NOTICE, "new connection");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return -1;
    }
    if (!common::connect_with_timeout(sockfd, (struct sockaddr*)&_servaddr,
                _servaddr_len, _option->connection_timeout)) {
        return -1;
    }

    _connection_pool[sockfd] = {pthread_self(), true, 0};
    return sockfd;
}

int Channel::get_connection()
{
    auto mutex_scope = common::makeScopeGuard(
        [this] () {
            if (pthread_mutex_lock(&_mutex) != 0) {
                LOG(ALERT, "lock mutex error: %s", strerror(errno));
                pthread_exit((void*)NULL);
            }
        },
        [this] () {pthread_mutex_unlock(&_mutex);}
    );

    ConnectionMap::iterator it;
    for (it = _connection_pool.begin(); it != _connection_pool.end(); ++it) {
        LOG(NOTICE, "find old connection");
        ConnectionInfo& cinfo = it->second;
        if (cinfo.is_idle) {
            LOG(NOTICE, "is idle");
            // TODO: check if server close
            cinfo.is_idle = false;
            break;
        }
    }
    if (it != _connection_pool.end()) {
        LOG(NOTICE, "reuse connection");
        return it->first;
    }

    int sockfd = new_connection();
    if (sockfd != -1) {
        _connection_pool[sockfd].is_idle = false;
    }
    return sockfd;
}

bool Channel::release_connection(int sockfd)
{
    auto mutex_scope = common::makeScopeGuard(
        [this] () {
            if (pthread_mutex_lock(&_mutex) != 0) {
                LOG(ALERT, "lock mutex error: %s", strerror(errno));
                pthread_exit((void*)NULL);
            }
        },
        [this] () {pthread_mutex_unlock(&_mutex);}
    );

    LOG(NOTICE, "release connection");
    //_connection_pool[sockfd].is_idle = true;
    ConnectionMap::iterator it = _connection_pool.find(sockfd);
    if (it != _connection_pool.end()) {
        ConnectionInfo& cinfo = it->second;
        cinfo.is_idle = true;
    }
    return true;
}

bool Channel::close_connection(int sockfd)
{
    auto mutex_scope = common::makeScopeGuard(
        [this] () {
            if (pthread_mutex_lock(&_mutex) != 0) {
                LOG(ALERT, "lock mutex error: %s", strerror(errno));
                pthread_exit((void*)NULL);
            }
        },
        [this] () {pthread_mutex_unlock(&_mutex);}
    );

    LOG(NOTICE, "close connection");
    close(sockfd);
    ConnectionMap::iterator it = _connection_pool.find(sockfd);
    if (it != _connection_pool.end()) {
        _connection_pool.erase(it);
        return true;
    }
    return false;
}

void Channel::CallMethod(const google::protobuf::MethodDescriptor* method,
        google::protobuf::RpcController* controller,
        const google::protobuf::Message* request,
        google::protobuf::Message* response,
        google::protobuf::Closure* done)
{
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
        // in sync mode, join it after pthread_create
        pthread_join(thid, NULL);
        return;
    }
    else {
        _async_thread_ids.push_back(thid);
        // just return. user should guarantee channel::~Channel() before all thread_exit or return.
        // you can use channel_join before return main(), or construct channel in a big scope.
        return;
    }
}

bool Channel::channel_join()
{
    int error = 0;
    std::for_each(_async_thread_ids.begin(), _async_thread_ids.end(),
        [&error] (pthread_t& thid) {
            if (pthread_join(thid, NULL) != 0) {
                LOG(ALERT, "join thread[%lu] error");
                error++;
            } else {
                LOG(INFO, "join thread[%lu] success");
            }
        }
    );
    _async_thread_ids.clear();
    return (error == 0) ? true : false;
}

/****************************************
 * pthread start_routine
 *     ChannelOperateParams
 *     rpc_call
 ****************************************/
static void rpc_call_core(ChannelOperateParams* params)
{
    Channel* channel = params->channel;
    Controller* cntl = params->cntl;
    ngxplus::IOBuf* msg = cntl->iobuf();

    int sockfd = channel->get_connection();
    if (sockfd == -1) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("get connect error");
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
                channel->close_connection(sockfd);
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
                channel->close_connection(sockfd);
                return cntl->finalize();
            }
            if (current_recv_len == 0) {
                recv_eof = true;
            }
            ParseResult parse_result =  default_protocol.parse(cntl, recv_eof);
            if (parse_result == PARSE_DONE) {
                // run here only if the parse is DONE, otherwise the connection is closed
                recv_eof ? channel->close_connection(sockfd) : channel->release_connection(sockfd);
                // process the response
                cntl->set_state(RPC_SESSION_PROCESSING);
                return default_protocol.process_response(cntl);
            } else if (parse_result == PARSE_INCOMPLETE) {
                remain_len -= current_recv_len;
                recv_buf += current_recv_len;
            } else {
                // error
                cntl->set_result(RPC_INNER_ERROR);
                cntl->set_result_text("parse response error");
                channel->close_connection(sockfd);
                return cntl->finalize();
            }
        }
    }

    return;
}

void* rpc_call(void* arg)
{
    ChannelOperateParams* params = static_cast<ChannelOperateParams*>(arg);
    google::protobuf::Closure* done = params->done;
    // process channel state, reuse, joinable calls

    rpc_call_core(params);
    if (done) {
        done->Run();
    }

    delete params;
    return NULL;
}

}
