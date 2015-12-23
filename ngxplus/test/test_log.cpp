#include "log.h"
#include "info_log_context.h"

int main()
{
    const char* name = "zmkeil";
    const char* hello = "hello";

    ngxplus::Log log;
    log.init("t1.log");

    ngxplus::Log log2;
    log2.init("t1.log");

    log.comlog_write(0, "hello %s", name);

    log2.comlog_write(0, "world %s", hello);

    ngxplus::InfoLogContext::get_context()->log(NGX_LOG_LEVEL_EMERG, "info_log_context");

    LOG(NGX_LOG_LEVEL_ALERT, "MACRO LOG");

    return 0;
}
