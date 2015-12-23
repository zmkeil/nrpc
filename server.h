
/***********************************************
  File name		: server.h
  Create date	: 2015-12-02 22:04
  Modified date : 2015-12-18 02:48
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

#define NOTICE NGX_LOG_NOTICE
#define WARN NGX_LOG_WARN
#define ERR NGX_LOG_ERR
#define ALERT NGX_LOG_ALERT
#define EMERG NGX_LOG_EMERG

namespace nrpc {

class Server {
public:
	Server();
	~Server();

    // new ServiceSet, and push_back to service_sets
    ServiceSet* push_service_set(const std::string& str_address
                    /*host:port:bind:wildcard:so_keepalive*/);

    // Start server
    int start();

	// Stop server
	int stop();

	// Join all processor after server.stop()
	int join();

public:
    static ngx_log_t* _s_error_log;

private:
    std::vector<ServiceSet*> _service_sets;
};

}
#endif
