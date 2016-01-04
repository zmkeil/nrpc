extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
}
#include <google/protobuf/service.h>

namespace nrpc {

struct ChannelOption
{
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

private:
    bool connect_with_timeout(int socket, struct sockaddr* addr, socklen_t addr_len, int timeout);

private:
    const char* _server_ip;
    int _server_port;
    struct sockaddr_in _servaddr;
    socklen_t _servaddr_len;
    const ChannelOption* _option;
};

}
