#include <iostream>
#include <string>
#include "ngx_nrpc_module.h"
#include "server.h"
#include "info_log_context.h"

int main()
{
    nrpc::Server server;

    LOG(NGX_LOG_LEVEL_NOTICE, "init server success, now we can use Components based on nginx");

    nrpc::ServiceSet* service_set = server.push_service_set("*:8833");
    service_set = server.push_service_set("*:8899");

    // service_set.add_service(NULL);

    nrpc::ServerOption option;
    server.start(&option);

    return 0;
}
