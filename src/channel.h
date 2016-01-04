#include <google/protobuf/service.h>

namespace nrpc {

class Channel : public google::protobuf::RpcChannel
{
public:
    Channel(std::string& server_address);
    virtual ~Channel();

    void CallMethod(const MethodDescriptor* method,
                    RpcController* controller,
                    const Message* request,
                    Message* response,
                    Closure* done);

private:
    std::string _server_ip;
    int _server_port;
};

}
