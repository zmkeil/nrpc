#include "channel.h"
#include "controller.h"

#include "xx.pb.h"

int main()
{
    nrpc::NrpcChannel channel("127.0.0.1:9988");

    nrpc::NrpcController controller;
    // controller.set_timeout(3);

    xxRequest req;
    xxResponse resp;
    xxserverStub stub(channel);
    // this will call _channel.CallMethod(methoddes, cntl, req, resp, NULL)
    stub.xxmethod(&controller, &req, &resp, NULL);

    // do something with resp

    return 0;
}
