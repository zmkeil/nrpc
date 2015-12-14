#include <iostream>
#include <string>
#include "ngx_nrpc_module.h"
#include "server.h"

int main(int argc, char** argv)
{
    nrpc::Server server;
    std::cout << "push service set" << std::endl;
    nrpc::ServiceSet* service_set = server.push_service_set("*:8833");

    service_set = server.push_service_set("*:8899");

    // service_set.add_service(NULL);

    server.start(argc, argv);
    return 0;
}
