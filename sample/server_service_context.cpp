#include <iostream>
#include <sstream>
#include <string>
#include "server.h"
#include "controller.h"
#include "echo.pb.h"
#include "echo_context.h"
#include "util.h"

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
		char p[1024] = {0};
		snprintf(p, 1024, ", id:%d, number:%d", request->persons(0).id(), request->persons(0).number());
		std::string str(p);
        response->set_res(request->msg() + " " + *context->comments() + str);
        // 2.log the session local information
		context->set_session_field("Recv \"" + request->msg() + "\"");
        context->set_session_field("Echo \"" + request->msg() + "\"");
		//context->set_session_field("PersonC " + request->persons_size());
		//context->set_session_field("Person[0].id " + request->persons(0).id());
		//context->set_session_field("Person[0].number " + request->persons(0).number());
		LOG(ERROR, "PersonC %d", request->persons_size());
		LOG(ERROR, "Person[0].id] %d", request->persons(0).id());
		LOG(ERROR, "Person[0].number %d", request->persons(0).number());

		return done->Run();
	}
};

int main()
{
    sample::server_side_config_log();

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
