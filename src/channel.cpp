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
#include <common_head.h>
#include <network_util.h>
#include <comlog/info_log_context.h>
#include <io/iobuf_zero_copy_stream.h>
#include <ngxplus_iobuf.h>
#include "controller.h"
#include "protocol.h"
#include "channel.h"
#include "connection_pool.h"

namespace nrpc {

Channel::Channel() : _option(nullptr), _connection_pool(nullptr)
{
}

Channel::~Channel()
{
    if (_connection_pool) {
        delete _connection_pool;
    }
}

bool Channel::init(const char* server_addr, int port, const ChannelOption* option)
{
    _connection_pool = new ConnectionPool();
    if (!_connection_pool->init(server_addr, port, option->connection_timeout)) {
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
    Controller* cntl = static_cast<Controller*>(controller);
    // init session protocol
    cntl->set_state(RPC_SESSION_SENDING);
    cntl->set_response(response);
    cntl->set_protocol(NRPC_PROTOCOL_DEFAULT_NUM);

    ngxplus::NgxplusIOBuf* msg = new ngxplus::NgxplusIOBuf();
    cntl->set_iobuf(msg);
    int bytes = default_protocol.pack_request(msg, method, cntl, *request);
    if (bytes == -1) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("pack error");
        return cntl->finalize();
    }
	// all the following logic runs in the pthread.

    // so channel must be deconstructed after rpc_call
    // In sync mode, this usually is not problem, In async mode, be carefully
    ChannelOperateParams* channel_operate_params = new ChannelOperateParams(this, done, _option->max_retry_time);
    cntl->set_channel_operate_params(channel_operate_params);
    pthread_t thid;
    if (pthread_create(&thid, NULL/*joinable*/, &rpc_call, cntl) != 0) {
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

bool Channel::channel_join(bool close_all)
{
	// just close the dropped connection, the establish connection can be reused
	// again when launching new rpc_call later
	_connection_pool->close_connection(close_all);

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
void rpc_call_core(Controller* cntl)
{
    ChannelOperateParams* params = cntl->channel_operate_params();
    Channel* channel = params->channel;
    ngxplus::NgxplusIOBuf* msg = cntl->iobuf();
    msg->read_point_cache();

    int sockfd = channel->connection_pool()->reuse_connection();
    if ((sockfd == -1) && ((sockfd = channel->connection_pool()->new_connection()) == -1)) {
        cntl->set_result(RPC_INNER_ERROR);
        cntl->set_result_text("get connection error");
        return cntl->finalize();
    }
	cntl->set_client_sockfd(sockfd);

    char *send_buf;
    char *recv_buf;
    int size;
    int remain_len;
    common::IOBufAsZeroCopyInputStream zero_in_stream(msg);
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
    common::IOBufAsZeroCopyOutputStream zero_out_stream(msg);
    while (zero_out_stream.Next((void**)&recv_buf, &size)) {
        if (size == 0) {
            continue;
        }
        // backword _byte, but not _buf
        msg->rebyte(0 - size);
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
            // forward _byte, but not _buf
            msg->rebyte(current_recv_len);
            //printf("RECV_BUF_LEN: %d\n", current_recv_len);
            //for (int i = 0; i < current_recv_len; i++) {
            //    printf("RECV_BUF: %X\n", recv_buf[i]);
            //}
            //printf("\n");
            ParseResult parse_result =  default_protocol.parse(cntl, recv_eof);
            if (parse_result == PARSE_DONE) {
                // Immediately release the connection if not recv_eof, else drop it later
				recv_eof ? : channel->connection_pool()->release_connection(sockfd);
				cntl->set_client_recv_eof(recv_eof);
                // process the response
                cntl->set_state(RPC_SESSION_PROCESSING);
                return default_protocol.process_response(cntl);
            } else if (parse_result == PARSE_INCOMPLETE) {
                remain_len -= current_recv_len;
                recv_buf += current_recv_len;
            } else {
                // error
                cntl->set_result(RPC_INNER_ERROR);
                cntl->set_result_text("parse response meta error");
                return cntl->finalize();
            }
        }
    }

    return;
}

void* rpc_call(void* arg)
{
    Controller* cntl = static_cast<Controller*>(arg);
    ChannelOperateParams* params = cntl->channel_operate_params();
    google::protobuf::Closure* done = params->done;
    // process channel state, reuse, joinable calls

    rpc_call_core(cntl);
    if (done) {
        done->Run();
    }

//	if (cntl->iobuf()) {
//		delete cntl->iobuf();
//	}
//    delete cntl;
    return NULL;
}

}
