#ifndef NRPC_SERVICE_CONTEXT_H
#define NRPC_SERVICE_CONTEXT_H

namespace nrpc {

// 1.session local context, set serverOption.service_context_factory before server.start(),
// here user can fill it with necessary field which will be used in every rpc_session by
// calling cntl->service_context()
// 2.also we can fill it with any fields in rpc_session_cycle by self_defined apis,
// and lastly, we can log these fields by implement build_log()
class ServiceContext
{
public:
    virtual ~ServiceContext() {}
    virtual void build_log(std::string* log) = 0;
};

class ServiceContextFactory
{
public:
    virtual ServiceContext* create_context() = 0;
    virtual void destory_context(ServiceContext* ctx) {
        delete ctx;
    }
};

}
#endif
