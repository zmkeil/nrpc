
/***********************************************
  File name		: test_protocol.cpp
  Create date	: 2015-12-28 23:57
  Modified date : 2015-12-29 01:20
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include <google/protobuf/text_format.h>
#include "rpc_session.h"
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
    nrpc::RpcSession* mock_session = new nrpc::RpcSession(NULL/*ngx_connection_t**/);
    nrpc::ServiceSet* mock_service_set = new nrpc::ServiceSet();
    mock_service_set->add_service(&service);
    mock_session->set_service_set(mock_service_set);

    // mock determine protocol
    mock_session->set_protocol(PROTOCOL_NUM);
    nrpc::Protocol* protocol = mock_session->protocol();

    mock_session->protocol()->parse(&iobuf, mock_session, true);
    mock_session->protocol()->process_request(mock_session, &iobuf);

    return 0;
}
