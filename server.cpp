
/***********************************************
  File name		: server.cpp
  Create date	: 2015-12-02 22:46
  Modified date : 2015-12-04 00:41
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "server.h"
#include "service_set.h"
#include "ngx_nrpc_module.h"

namespace nrpc {

Server::Server()
{
}

Server::~Server()
{
}

ServiceSet* Server::push_service_set(const std::string& str_address)
{
    ServiceSet* service_set = new ServiceSet(this);
    if (ngx_nrpc_add_listen(str_address, service_set) != 0) {
        delete service_set;
        return NULL;
    }

    _service_sets.push_back(service_set);
    return service_set;
}

int Server::start()
{
    // initialize input_handler, only one now. 
	// TODO: support mutiple-policys, realize an adaptor
//	InputMessageHandler handler;
//	handler.parse = parse_rpc_message;
//	handler.process = process_rpc_request;
//	handler.name = "nrpc";
//	handler.arg = this;

	return 0;
}

}
