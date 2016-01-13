#include <common.h>
#include <info_log_context.h>
#include "channel.h"
#include "controller.h"

#include "echo.pb.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: ./client <IP>\n");
        return -1;
    }
    // for test
    std::string error_log_file("client_error.log");
    ngxplus::InfoLogContext::set_log_file(error_log_file);
    common::COMMON_TEST_FLAG = 1;

    nrpc::Channel channel;
    nrpc::ChannelOption option;
    if (!channel.init(argv[1], 8899, &option)) {
        return -1;
    }
    nrpc::EchoService_Stub stub(&channel);

/*      this will call _channel.CallMethod(methoddes, cntl, req, resp, NULL)
 *        1.pack request
 *        2.send request buf
 *        3.recv response
 *        4.parse response
 */

    char req_msg[2] = "A";
    for (int i = 0; i < 10; i++) {
        // the three element of RPC
        nrpc::Controller* cntl = new nrpc::Controller();
        nrpc::EchoResponse* resp = new nrpc::EchoResponse();
        nrpc::EchoRequest req;
        req.set_msg(req_msg);
        stub.Echo(cntl, &req, resp, NULL);
        if (cntl->Failed()) {
            std::cout << "rpc failed: " << cntl->ErrorText().c_str() << std::endl;
        } else {
            std::cout << resp->res() << std::endl;
        }

        // for next rpc
        req_msg[0]++;
    }

    return 0;
}

