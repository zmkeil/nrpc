#ifndef NRPC_SERVICE_CONTEXT_LOG_H
#define NRPC_SERVICE_CONTEXT_LOG_H

#include "log.h"
#include "controller.h"

namespace nrpc {

class ServiceContextLog;
extern ServiceContextLog* access_context_log;

class ServiceContextLog
{
public:
    static ServiceContextLog* get_context() {
        if (!access_context_log) {
            access_context_log = new ServiceContextLog();
        }
        return access_context_log;
    }

public:
    void set_start_time(time_t rawtime);
    void set_rt(int rt_us);
    void set_ret_code(RPC_RESULT result);
    void set_ret_text(RPC_SESSION_STATE state, const char* result_text);

    void push_service_context(ServiceContext* service_context) {
        service_context->build_log(&_log_content);
    }

    void clear() {
        _log_content.clear();
    }
    void flush() {
        _log.comlog_write(NOTICE, "%s", _log_content.c_str());
    }

private:
    ServiceContextLog() {
        _log.init(SERVICE_LOG_NAME_DEFAULT, DEBUG);
        _log_content.reserve(SERVER_LOG_MAX_LEN);
        _log_content.clear();
    }

    void insert_delimiter();

private:
    ngxplus::Log _log;
    // ngx core is single thread, so this is safe
    std::string _log_content;

private:
    static const std::string SERVICE_LOG_NAME_DEFAULT;
    static const int SERVER_LOG_MAX_LEN = 1024;
    static const char* _delimiter;
};

}
#endif
