
/***********************************************
  File name		: server.cpp
  Create date	: 2015-12-02 22:46
  Modified date : 2015-12-31 02:30
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include <iostream>
#include "server.h"
#include "service_set.h"
#include "ngx_nrpc_module.h"

namespace nrpc {

/* 
 * Now server must be single,
 * because ngx_nrpc_module use global
 *     ngx_extern_modules and nrpc_listens
 */ 
Server::Server()
{
    // add nrpc module, in server.construct
    ngx_extern_modules[0] = &ngx_nrpc_module;
    ngx_extern_module_names[0] = (u_char*)"nrpc";
    // clear all listen before
    ngx_nrpc_clear_listen();
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

int Server::start(ServerOption* option)
{
    _option = option;

/*     if (ngx_pre_init() != 0) {
 *         std::cout << "ngx_pre_init failed" << std::endl;
 *         return -1;
 *     }
 */
    // initialize input_handler, only one now. 
	// TODO: support mutiple-policys, realize an adaptor
//	InputMessageHandler handler;
//	handler.parse = parse_rpc_message;
//	handler.process = process_rpc_request;
//	handler.name = "nrpc";
//	handler.arg = this;

    ngx_start();
	return 0;
}


// get options
ServiceContext* Server::local_service_context()
{
    if (!_option->service_context_factory) {
        return nullptr;
    }
    return _option->service_context_factory->create_context();
}

int Server::read_timeout()
{
    return _option->read_timeout;
}

bool Server::is_connection_reuse()
{
    return _option->is_connection_reuse;
}

int Server::idle_timeout()
{
    return _option->idle_timeout;
}

int Server::write_timeout()
{
    return _option->write_timeout;
}

bool Server::get_concurrency()
{
	_option->max_concurrency--;
	if (_option->max_concurrency >= 0) {
		return true;
	}
	return false;
}

void Server::free_concurrency()
{
	_option->max_concurrency++;
}

}
