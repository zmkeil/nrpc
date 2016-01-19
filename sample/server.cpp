#include <iostream>
#include <string>
#include "server.h"
#include "controller.h"
#include "info_log_context.h"
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
		std::cout << "in Echo method" << std::endl;
        std::cout << "request msg: " << request->msg() << std::endl;
        response->set_res(request->msg());
		return done->Run();
	}

	virtual void Reflect(google::protobuf::RpcController* cntl_base,
			const nrpc::EchoRequest* request,
			nrpc::Student* student,
			google::protobuf::Closure* done) {
        nrpc::Controller* cntl = static_cast<nrpc::Controller*>(cntl_base);
		std::cout << "in Reflect method" << std::endl;
        std::cout << "request msg: " << request->msg() << std::endl;
        cntl->SetFailed("can't reflect from EchoRequest to Student");
        student->set_name(request->msg());
		return done->Run();
	}
};

int main()
{
    nrpc::Server server;
    EchoServiceImpl service;

    nrpc::ServiceSet* service_set = server.push_service_set("*:8899");
    service_set->add_service(&service);

    nrpc::ServerOption option;
    server.start(&option);

    return 0;
}
