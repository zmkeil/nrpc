#include <iostream>
#include <string>
#include "ngx_nrpc_module.h"
#include "server.h"
#include "log.h"

int main(int argc, char** argv)
{
    nrpc::Server server;
    if (!server.init()) {
        return -1;
    }

    LOG(NOTICE, 0, "init server success, now we can use Components based on nginx");

    nrpc::ServiceSet* service_set = server.push_service_set("*:8833");
    service_set = server.push_service_set("*:8899");

    // service_set.add_service(NULL);

    server.start();
    return 0;
}
