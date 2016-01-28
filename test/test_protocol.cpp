
/***********************************************
  File name		: test_protocol.cpp
  Create date	: 2015-12-28 23:57
  Modified date : 2016-01-01 00:42
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include <google/protobuf/text_format.h>
#include <comlog/info_log_context.h>
#include <io/iobuf_zero_copy_stream.h>
#include <ngxplus_iobuf.h>

#include "controller.h"
#include "protocol.h"
#include "echo.pb.h"

class EchoServiceImpl : public nrpc::EchoService {
public:
	EchoServiceImpl() {};
	virtual ~EchoServiceImpl() {};
	virtual void Echo(google::protobuf::RpcController* cntl_base, 
			const nrpc::EchoRequest* request, 
			nrpc::EchoResponse* response, 
			google::protobuf::Closure* done) {
        (void) cntl_base;
        // don't done->run(), call default_send_rpc_response(false) for mock test
        (void) done;
		std::cout << "in server Echo method----" << std::endl;
        std::cout << "request msg: " << request->msg() << std::endl;
        response->set_res(request->msg() + " to client");
		return;
	}
};

#define PROTOCOL_NUM 0

int main()
{
    EchoServiceImpl service;
    // a mock iobuf, through the entire procedure
    ngxplus::NgxplusIOBuf iobuf;

    /******************************
     * client side
     *****************************/
    // mock a request
    const google::protobuf::ServiceDescriptor* psdes = service.GetDescriptor();
	const google::protobuf::MethodDescriptor *pmdes = psdes->method(0);
    nrpc::EchoRequest req;
    req.set_msg("hello");

    // mock the channel's CallMethod
    nrpc::Controller* client_cntl = new nrpc::Controller();
    client_cntl->set_protocol(PROTOCOL_NUM);
    client_cntl->set_iobuf(&iobuf);
    nrpc::EchoResponse client_resp;
    client_cntl->set_response(&client_resp);
    // pack the request 
    nrpc::g_rpc_protocols[PROTOCOL_NUM]->pack_request(&iobuf, pmdes, client_cntl, req);


    /******************************
     * server side
     *****************************/
    nrpc::Controller* server_cntl = new nrpc::Controller();
    // determin protocol
    server_cntl->set_protocol(PROTOCOL_NUM);
    server_cntl->set_iobuf(&iobuf);
    nrpc::ServiceSet* mock_service_set = new nrpc::ServiceSet();
    mock_service_set->add_service(&service);
    server_cntl->set_service_set(mock_service_set);

    // process request from iobuf
    nrpc::Protocol* protocol = server_cntl->protocol();
    protocol->parse(server_cntl, true);
    protocol->process_request(server_cntl);
    // just pack the response without real_send
    nrpc::default_send_rpc_response(server_cntl, false);

    /******************************
     * client side, process response
     *****************************/
    // recv data and parse
    protocol->parse(client_cntl, true);
    protocol->process_response(client_cntl);
    // do something with client_resp
    std::cout << "in client side----" << std::endl;
    std::cout << "response res: " << client_resp.res() << std::endl;

    return 0;
}
