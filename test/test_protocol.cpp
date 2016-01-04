
/***********************************************
  File name		: test_protocol.cpp
  Create date	: 2015-12-28 23:57
  Modified date : 2016-01-01 00:42
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include <google/protobuf/text_format.h>
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
		std::cout << "in Echo method" << std::endl;
        std::cout << "request msg: " << request->msg() << std::endl;
		return;
	}
};

#define PROTOCOL_NUM 0

int main()
{
    EchoServiceImpl service;

    // pack request to iobuf
    const google::protobuf::ServiceDescriptor* psdes = service.GetDescriptor();
	const google::protobuf::MethodDescriptor *pmdes = psdes->method(0);
    nrpc::EchoRequest req;
    req.set_msg("hello");
    ngxplus::IOBuf iobuf;
    nrpc::g_rpc_protocols[PROTOCOL_NUM]->pack_request(&iobuf, pmdes, NULL, req);


    // process request from iobuf
    nrpc::Controller* mock_cntl = new nrpc::Controller();
    mock_cntl->set_iobuf(&iobuf);
    nrpc::ServiceSet* mock_service_set = new nrpc::ServiceSet();
    mock_service_set->add_service(&service);
    mock_cntl->set_service_set(mock_service_set);

    // mock determine protocol
    mock_cntl->set_protocol(PROTOCOL_NUM);
    nrpc::Protocol* protocol = mock_cntl->protocol();

    protocol->parse(mock_cntl, true);
    protocol->process_request(mock_cntl);

    return 0;
}
