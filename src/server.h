
/***********************************************
  File name		: server.h
  Create date	: 2015-12-02 22:04
  Modified date : 2016-01-04 23:24
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/
#ifndef NRPC_SERVER_H
#define NRPC_SERVER_H
extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>
}
#include <vector>
#include <string>
#include "service_set.h"

namespace nrpc {

struct ServerOption {
    ServerOption() :
            read_timeout(3/*s*/),
            max_concurrency(8) {}

    int read_timeout;
    int max_concurrency;
};

class Server {
public:
	Server();
	~Server();

    // new ServiceSet, and push_back to service_sets
    ServiceSet* push_service_set(const std::string& str_address
                    /*host:port:bind:wildcard:so_keepalive*/);

    // Start server
    int start(ServerOption* option);

	// Stop server
	int stop();

	// Join all processor after server.stop()
	int join();


    // get options
    int read_timeout();

private:
    ServerOption* _option;
    std::vector<ServiceSet*> _service_sets;
};

}
#endif
