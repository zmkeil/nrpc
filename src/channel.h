extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
}
#include <map>
#include <google/protobuf/service.h>

namespace nrpc {

class Controller;

struct ChannelOption
{
    ChannelOption() : connection_timeout(5),
    send_timeout(5),
    read_timeout(5),
    max_retry_time(3) {}
    int connection_timeout;
    int send_timeout;
    int read_timeout;
    int max_retry_time;
};

struct ConnectionInfo
{
    pthread_t thid;
    bool is_idle;
    int reuse_time;
};

typedef std::map<int, ConnectionInfo> ConnectionMap;

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
    bool channel_join();

    // Not implemented yet
    // bool channel_cancel();

public:
    // the connection_pool operation
    // this function without sync lock, never call it in pthread. it is called:
    //   1.when channel.init with the flag_reserve_connection(not implemented yet).
    //   2.by get_connection if there is no idle connection
    int new_connection();
    // be called when launch a rpc_call, return sockfd
    int get_connection();
    // release a connection after the last_rpc.parse DONE
    bool release_connection(int sockfd);
    // close a connection when:
    // 1.any error (read/recv/parse) occurs in this connection, avoid dirty data
    // 2.server close the connection. there are two point to check if the connection is closed by server:
    //   I)check eof when recv response, it works if server.is_connection_reuse is false
    //   II)after get_connection, TODO: this check is not guaranteed absolute effective because the FIN from
    //   server may come just after get_connection, so if READ_ERROR returns, we should try again with
    //   a new_connection()
    bool close_connection(int sockfd);

public:
    struct sockaddr* servaddr() {
        return (struct sockaddr*)&_servaddr;
    }
    socklen_t servaddr_len() {
        return _servaddr_len;
    }
    const ChannelOption* option() {
        return _option;
    }
    pthread_mutex_t* mutex() {
        return &_mutex;
    }

private:
    const ChannelOption* _option;

    // Don't implement something like lb(loadbalance), just mutil-connections over one channel(the
    // same server-end ip:port).
    const char* _server_ip;
    int _server_port;
    struct sockaddr_in _servaddr;
    socklen_t _servaddr_len;

    // Implement a simple connection-pool, a connection can only be used by one pthread(rpc_call)
    // in a time, all the information are recorded in _connection_pool, and be processed in pre-thread,
    // so the function get_connection()/new_connection()/release_connection()/close_connection() must
    // be thread-safe.
    pthread_mutex_t _mutex;
    ConnectionMap _connection_pool;

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
    ChannelOperateParams(Channel* channel, Controller* cntl, google::protobuf::Closure* done) :
        channel(channel), cntl(cntl), done(done) {}
    Channel* channel;
    Controller* cntl;
    google::protobuf::Closure* done;
};

void* rpc_call(void* arg);

}
