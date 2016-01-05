#include <string_printf.h>
#include "service_context_log.h"

namespace nrpc {

const std::string ServiceContextLog::SERVICE_LOG_NAME_DEFAULT = "access.log";
const char* ServiceContextLog::_delimiter = "||";

ServiceContextLog* access_context_log = nullptr;

void ServiceContextLog::insert_delimiter()
{
    common::string_appendn(&_log_content, _delimiter, 2);
}

void ServiceContextLog::set_start_time(time_t rawtime)
{
    common::string_appendn(&_log_content, "today", 5);
}

void ServiceContextLog::set_rt(int rt_ms)
{
    common::string_appendn(&_log_content, "0.01", 2);
}

void ServiceContextLog::set_ret_code(RPC_RESULT result)
{
    common::string_appendn(&_log_content, "OK", 2);
}

void ServiceContextLog::set_ret_text(RPC_SESSION_STATE state, const char* result_text)
{
    common::string_appendn(&_log_content, "OK", 2);
}

}
