#include <google/protobuf/service.h>

namespace nrpc {

class NrpcChannel : public google::protobuf::RpcChannel
{
public:
    NrpcChannel(std::string& server_address);
    virtual ~NrpcChannel();

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
