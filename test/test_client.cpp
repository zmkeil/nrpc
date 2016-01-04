#include "channel.h"
#include "controller.h"

#include "echo.pb.h"

int main()
{
    nrpc::Channel channel;
    nrpc::ChannelOption option;
    if (!channel.init("127.0.0.1", 9988, &option)) {
        return -1;
    }

    nrpc::Controller cntl(NULL);

    nrpc::EchoRequest req;
    nrpc::EchoResponse resp;
    nrpc::EchoService_Stub stub(&channel);
    // this will call _channel.CallMethod(methoddes, cntl, req, resp, NULL)
    stub.Echo(&cntl, &req, &resp, NULL);

    // do something with resp
    std::cout << resp.res() << std::endl;

    return 0;
}
