
#include "ngx_nrpc_module.h"
#include "server.h"

int main()
{
    nrpc::Server server;
    nrpc::ServiceSet* service_set = server.push_service_set("8833");

    // service_set.add_service(NULL);

    server.start();
    return 0;
}
