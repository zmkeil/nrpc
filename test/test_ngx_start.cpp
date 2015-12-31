#include <iostream>
#include <string>
#include "ngx_nrpc_module.h"
#include "server.h"
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
		std::cout << "in Echo method" << std::endl;
        std::cout << "request msg: " << request->msg() << std::endl;
        LOG(NGX_LOG_LEVEL_NOTICE, "in Echo method");
		return;
	}
};

int main()
{
    nrpc::Server server;
    EchoServiceImpl service;

    nrpc::ServiceSet* service_set = server.push_service_set("*:8833");
    service_set = server.push_service_set("*:8899");
    service_set->add_service(&service);

    // service_set.add_service(NULL);

    nrpc::ServerOption option;
    server.start(&option);

    return 0;
}
