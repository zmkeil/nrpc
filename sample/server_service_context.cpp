#include <iostream>
#include <string>
#include "server.h"
#include "controller.h"
#include "info_log_context.h"
#include "echo.pb.h"
#include "echo_context.h"

class EchoServiceImpl : public nrpc::EchoService {
public:
	EchoServiceImpl() {};
	virtual ~EchoServiceImpl() {};
	virtual void Echo(google::protobuf::RpcController* cntl_base,
			const nrpc::EchoRequest* request,
			nrpc::EchoResponse* response,
			google::protobuf::Closure* done) {
        nrpc::Controller* cntl = static_cast<nrpc::Controller*>(cntl_base);
        // 1.get global data to this session
        EchoContext* context = static_cast<EchoContext*>(cntl->service_context());
        response->set_res(request->msg() + " " + *context->comments());
        // 2.log the session local information
		context->set_session_field("Recv \"" + request->msg() + "\"");
        context->set_session_field("Echo \"" + request->msg() + "\"");

		return done->Run();
	}
};

int main()
{
    nrpc::Server server;
    EchoServiceImpl service;

    nrpc::ServiceSet* service_set = server.push_service_set("*:8899");
    service_set->add_service(&service);

    std::string comment("from echo service");
    EchoContextFactory factory(&comment);
    nrpc::ServerOption option;
    option.service_context_factory = &factory;
    server.start(&option);

    return 0;
}
