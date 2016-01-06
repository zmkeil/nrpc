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
    nrpc::EchoService_Stub stub(&channel);

    // the three element of RPC
    nrpc::Controller cntl;
    nrpc::EchoRequest req;
    nrpc::EchoResponse resp;
    nrpc::Student student;
    req.set_msg("hello client");

    // this will call _channel.CallMethod(methoddes, cntl, req, resp, NULL)
    //   1.pack request
    //   2.send request buf
    //   3.recv response
    //   4.parse response
    stub.Echo(&cntl, &req, &resp, NULL);
    // do something with resp
    if (cntl.Failed()) {
        LOG(ALERT, "rpc failed: %s", cntl.ErrorText().c_str());
        return -1;
    }
    std::cout << resp.res() << std::endl;

    stub.Reflect(&cntl, &req, &student, NULL);
    // do something with resp
    if (cntl.Failed()) {
        LOG(ALERT, "rpc failed: %s", cntl.ErrorText().c_str());
    }
    std::cout << student.name() << std::endl;

    return 0;
}
