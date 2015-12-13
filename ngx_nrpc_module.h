
struct ngx_nrpc_listen_s {
    nrpc::ServiceAddress service_address;
    nrpc::ServiceSet *service_set;
};
typedef struct ngx_nrpc_listen_s ngx_nrpc_listen_t;


int ngx_nrpc_add_listen(const std::string& str_address, nrpc::ServiceSet* service_set);

