#ifndef NRPC_NGX_NRPC_MODULE_H
#define NRPC_NGX_NRPC_MODULE_H
extern "C" {
#include <nginx.h>
}
#include "service_set.h"
#include "protocol.h"

extern ngx_module_t ngx_nrpc_module;

namespace nrpc {
int ngx_nrpc_clear_listen();

int ngx_nrpc_add_listen(const std::string& str_address, nrpc::ServiceSet* service_set);

}
#endif
