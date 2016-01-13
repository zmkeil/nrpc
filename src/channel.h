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
	ConnectionInfo() : thid(0), is_idle(true), reuse_time(0) {}
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
/*
 * these two function deal with network, may time-cost
 * without sync lock
 */
    // this function without sync lock. it is called:
    //   1.when channel.init with the flag_reserve_connection(NOT implemented yet).
    //   2.if reuse_connection() return -1, then new_connection()
	// why no sync_lock
	//   build a new connection is a time-cost operate, and this function doesn't 
	//   operate the _connection_pool
    int new_connection();
	// really close the connection from the _close_connection_queue, just use push_back()
	// and pop_back(), the thread-safe is guaranteed by STL:vector
	void close_connection();

/*
 * these three function deal with _connection_pool
 * must under sync lock
 */
    // the connection_pool operation
    // be called when launch a rpc_call, return idle and clear sockfd from _connection_pool.
	// this function must be called under sync lock
    int reuse_connection();
    // release a connection after the last rpc parse DONE. if new connection, push
	// it into _connection_pool, otherwise modify its state. under sync lock.
    bool release_connection(int sockfd);
    // drop a connection when: under sync lock
    // 1.any error (read/recv/parse) occurs in this connection, avoid dirty data
    // 2.server close the connection. there are two point to check if the connection is closed by server:
    //   I)check eof when recv response. it works if server.is_connection_reuse is false
    //   II)after reuse_connection. this check is not guaranteed absolute effective because the FIN from
    //   server may come just after reuse_connection, so if READ_ERROR returns, we should try again with
    //   a new_connection()
	// NOTE: this function just move the socket to the drop_connection_queue, close_connection()
	// really close these connection. Now it is hooked at finalize_client()
    bool drop_connection(int sockfd);

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
    // so the function reuse_connection()/release_connection()/drop_connection() must be thread-safe.
    pthread_mutex_t _mutex;
    ConnectionMap _connection_pool;
	// dropped connection, be really closed asynchronously
	std::vector<int> _close_connection_queue/*sockfd*/;

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
