
/***********************************************
  File name		: test_protocol.cpp
  Create date	: 2015-12-28 23:57
  Modified date : 2015-12-29 01:20
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include <google/protobuf/text_format.h>
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

int main()
{
    EchoServiceImpl service;

    // pack request to iobuf
    const google::protobuf::ServiceDescriptor* psdes = service.GetDescriptor();
	const google::protobuf::MethodDescriptor *pmdes = psdes->method(0);
    nrpc::EchoRequest req;
    req.set_msg("hello");
    ngxplus::IOBuf iobuf;
    nrpc::default_protocol.pack_request(&iobuf, pmdes, NULL, req);

    // process request from iobuf
    nrpc::RpcSession* mock_session = new nrpc::RpcSession();
    nrpc::ServiceSet* mock_service_set = new nrpc::ServiceSet();
    mock_service_set->add_service(&service);
    mock_session->service_set = mock_service_set;
    nrpc::RpcMeta mock_meta;

    ngxplus::IOBufAsZeroCopyInputStream zero_in_stream(&iobuf);
    iobuf.cutn(21/*size of meta_bytes*/);
    mock_meta.ParseFromZeroCopyStream(&zero_in_stream);
    std::string mock_meta_string;
    google::protobuf::TextFormat::PrintToString(mock_meta, &mock_meta_string);
    std::cout << mock_meta_string << std::endl;
    iobuf.carrayon();

    nrpc::default_protocol.process_request(mock_session, mock_meta, &iobuf);

    return 0;
}
