#include "channel.h"
#include "controller.h"

#include "echo.pb.h"

int main()
{
    nrpc::Channel channel;
    nrpc::ChannelOption option;
    if (!channel.init("192.168.2.188", 8899, &option)) {
        return -1;
    }

    nrpc::Controller cntl;

    nrpc::EchoRequest req;
    req.set_msg("hello client");
    nrpc::EchoResponse resp;
    nrpc::EchoService_Stub stub(&channel);
    // this will call _channel.CallMethod(methoddes, cntl, req, resp, NULL)
    stub.Echo(&cntl, &req, &resp, NULL);

    // do something with resp
    std::cout << resp.res() << std::endl;

    return 0;
}
