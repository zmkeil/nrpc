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
#include "service_set.h"

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

    cout << "TEST standalone service ========" << endl;
	const google::protobuf::ServiceDescriptor* psdes = service.GetDescriptor();
	std::cout << "service name: " << psdes->name() << std::endl;
	int i = psdes->method_count();
	std::cout << "method_count: " << i << std::endl;

	const google::protobuf::MethodDescriptor *pmdes = psdes->method(0);
	std::cout << "method name: " << pmdes->name() << std::endl;
	std::cout << "method full name: " << pmdes->full_name() << std::endl;

    cout << "CallMethod: ";
	service.CallMethod(pmdes, NULL, NULL, NULL, NULL);

    cout << endl;
    cout << "TEST service_set ===============" << endl;
    nrpc::ServiceSet* service_set = new nrpc::ServiceSet();
    if (!service_set->add_service(&service)) {
        cout << "add_service error" << endl;
        return -1;
    }

    string dump_message;
    service_set->dump(&dump_message);
    cout << dump_message << endl;
	return 0;
}

