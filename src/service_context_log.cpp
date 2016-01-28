#include <common_head.h>
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
    char* localtime = ::asctime(::localtime(&rawtime));
    localtime[strlen(localtime) - 1] = '\0';
    common::string_appendn(&_log_content, localtime, strlen(localtime));
    insert_delimiter();
}

void ServiceContextLog::set_rt(int rt_us)
{
    common::string_appendf(&_log_content, "%.3f", (float)rt_us/1000);
    insert_delimiter();
}

void ServiceContextLog::set_ret_code(RPC_RESULT result)
{
    std::string ret_code_s;
    switch (result) {
        case RPC_READ_ERROR:
            ret_code_s = "READ_ERROR";
            break;
        case RPC_READ_TIMEOUT:
            ret_code_s = "READ_TIMEOUT";
            break;
        case RPC_PROCESS_ERROR:
            ret_code_s = "PROCESS_ERROR";
            break;
        case RPC_PROCESS_TIMEOUT:
            ret_code_s = "PROCESS_TIMEOUT";
            break;
        case RPC_SEND_ERROR:
            ret_code_s = "SEND_ERROR";
            break;
        case RPC_SEND_TIMEOUT:
            ret_code_s = "SEND_TIMEOUT";
            break;
        case RPC_INNER_ERROR:
            ret_code_s = "INNER_ERROR";
            break;
        case RPC_OK:
            ret_code_s = "OK";
            break;
        default:
            ret_code_s = "UNKOWN_ERROR";
            break;
    }
    //common::string_appendn(&_log_content, ret_code_s, strlen(ret_code_s));
    _log_content += ret_code_s;
    insert_delimiter();
}

void ServiceContextLog::set_ret_text(RPC_SESSION_STATE state, const char* result_text)
{
    if (result_text) {
        common::string_appendn(&_log_content, result_text, strlen(result_text));
    } else {
        std::string text;
        switch (state) {
            case RPC_SESSION_READING:
                text = "Unkown read failed";
                break;
            case RPC_SESSION_PROCESSING:
                text = "Unkown process failed";
                break;
            case RPC_SESSION_SENDING:
                text = "Unkown send failed";
                break;
            case RPC_SESSION_LOGING:
            case RPC_SESSION_OVER:
                text = "ok";
                break;
            default:
                text = "Unkown session state";
                break;
        }
        _log_content += text;
    }
    insert_delimiter();
}

}
