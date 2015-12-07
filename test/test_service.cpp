/***********************************************
  File name		: test_service.cpp
  Create date	: 2015-12-04 21:29
  Modified date : 2015-12-04 22:35
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include <stdlib.h>
#include <iostream>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include "echo.pb.h"

using namespace std;

class EchoServiceImpl : public test::nrpc::EchoService {
public:
	EchoServiceImpl() {};
	virtual ~EchoServiceImpl() {};
	virtual void Echo(google::protobuf::RpcController* cntl_base, 
			const test::nrpc::EchoRequest* request, 
			test::nrpc::EchoResponse* response, 
			google::protobuf::Closure* done) {
		std::cout << "in Echo method" << std::endl;
		return;
	}
};

int main(int argc, char** argv)
{
	EchoServiceImpl service;
	const google::protobuf::ServiceDescriptor* psdes = service.GetDescriptor();
	int i = psdes->method_count();
	std::cout << "method_count: " << i << std::endl;
	std::cout << "service name: " << psdes->name() << std::endl;

	const google::protobuf::MethodDescriptor *pmdes = psdes->method(0);
	std::cout << "method name: " << pmdes->name() << std::endl;
	std::cout << "method full name: " << pmdes->full_name() << std::endl;

	service.CallMethod(pmdes, NULL, NULL, NULL, NULL);

	return 0;
}

