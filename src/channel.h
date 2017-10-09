#ifndef NRPC_CHANNEL_H
#define NRCP_CHANNEL_H

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
}
#include <google/protobuf/service.h>

/*
 * For client side, implemented based on pthread.
 * In mobile apps, we can extract the client side code, include:
 *   1. channel.h/channel.cpp
 *   2. connection_pool.h/connection_pool.cpp
 *   3. controller.h/controller.h (client part)
 *   4. policy/xx_protocol.cpp (client part such as parse, process_response)
 */
namespace nrpc {

class Controller;
class ConnectionPool;

struct ChannelOption
{
    ChannelOption() : connection_timeout(5),
    send_timeout(5),
    read_timeout(5),
    max_retry_time(0) {}
    int connection_timeout;
    int send_timeout;
    int read_timeout;
    int max_retry_time;
};

class Channel : public google::protobuf::RpcChannel
{
public:
    Channel();
    virtual ~Channel();

    bool init(const char* server_addr, int port, const ChannelOption* option);

    void CallMethod(const google::protobuf::MethodDescriptor* method,
            google::protobuf::RpcController* controller,
            const google::protobuf::Message* request,
            google::protobuf::Message* response,
            google::protobuf::Closure* done);

    // do nothing but pthread_join(all thread)
    bool channel_join(bool close_all);

    // Not implemented yet
    // bool channel_cancel();

public:
    const ChannelOption* option() {
        return _option;
    }
    ConnectionPool* connection_pool() {
        return _connection_pool;
    }

private:
    const ChannelOption* _option;
    ConnectionPool* _connection_pool;

    // And the _async_thread_ids is the thread_id of the async calls, user can join the async
    // call by channel_join(), the function do nothing but pthread_join(all the threads), all
    // the record, update etc. operation are processed in pre-thread-runtime
    std::vector<pthread_t> _async_thread_ids;
};


/****************************************
 * pthread start_routine
 *     ChannelOperateParams
 *     rpc_call
 ****************************************/
struct ChannelOperateParams {
    ChannelOperateParams(Channel* channel, google::protobuf::Closure* done, int max_retry_time) :
        channel(channel), done(done), max_retry_time(max_retry_time) {}
    Channel* channel;
    google::protobuf::Closure* done;
    int max_retry_time;
};

void* rpc_call(void* arg);

}
#endif
