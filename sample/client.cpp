#include <common_head.h>
#include "channel.h"
#include "controller.h"
#include "echo.pb.h"
#include "util.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: ./client <IP>\n");
        return -1;
    }
    sample::client_side_config_log();

    nrpc::Channel channel;
    nrpc::ChannelOption option;
    if (!channel.init(argv[1], 8899, &option)) {
        return -1;
    }
    nrpc::EchoService_Stub stub(&channel);

    // the three element of RPC
    nrpc::Controller cntl;
    nrpc::EchoRequest req;
    nrpc::EchoResponse resp;
    nrpc::Student student;
    req.set_msg("hello client");

/*      this will call _channel.CallMethod(methoddes, cntl, req, resp, NULL)
 *        1.pack request
 *        2.send request buf
 *        3.recv response
 *        4.parse response
 */

    // 1.rpc call successly
    stub.Echo(&cntl, &req, &resp, NULL);
    if (cntl.Failed()) {
        printf("rpc failed: %s", cntl.ErrorText().c_str());
    }
    std::cout << resp.res() << std::endl;

    // 2.rpc call failed
    stub.Reflect(&cntl, &req, &student, NULL);
    if (cntl.Failed()) {
        printf("rpc failed: %s", cntl.ErrorText().c_str());
    }
    std::cout << student.name() << std::endl;

	// 3.rpc method not found at server side
	//   the server is builded with another echo.proto, !-!
    stub.Test(&cntl, &req, &student, NULL);
    if (cntl.Failed()) {
        printf("rpc failed: %s", cntl.ErrorText().c_str());
    }
    std::cout << student.name() << std::endl;

    return 0;
}
