extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
}
#include <google/protobuf/service.h>

namespace nrpc {

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

    bool channel_join();

    // Not implemented yet
    // bool channel_cancel();

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

private:
    const ChannelOption* _option;
    // don't implement something like lb(loadbalance), there is only one connection,
    // So it is easily to guarente thread-safe, we just need a _in_use to indicate if the
    // channel is in used.
    // And the _join_thread_ids is the thread_id of the async calls, user can join the async
    // call by channel_join()
    const char* _server_ip;
    int _server_port;
    struct sockaddr_in _servaddr;
    socklen_t _servaddr_len;

    bool _in_use;
    pthread_t _join_thread_ids;
};

}
