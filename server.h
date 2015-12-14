
/***********************************************
  File name		: server.h
  Create date	: 2015-12-02 22:04
  Modified date : 2015-12-14 00:11
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/
#ifndef NRPC_SERVER_H
#define NRPC_SERVER_H

#include <vector>
#include <string>
#include "service_set.h"

namespace nrpc {

class Server {
public:
	Server();
	~Server();

    // new ServiceSet, and push_back to service_sets
    ServiceSet* push_service_set(const std::string& str_address
                    /*host:port:bind:wildcard:so_keepalive*/);

    // Start server
    int start(int argc, char** argv);

	// Stop server
	int stop();

	// Join all processor after server.stop()
	int join();

private:
    std::vector<ServiceSet*> _service_sets;
};

}
#endif
