#include <comlog/info_log_context.h>
#include "ngxplus_log.h"

int main()
{
    LOG(ALERT, "test log with stderr_log");

    ngxplus::NgxplusLog* log1 = new ngxplus::NgxplusLog();
    log1->init();
    common::InfoLogContext::set_log((common::AbstractLog*)log1);
    LOG(NOTICE, "test log with ngxplus_log");

    ngxplus::NgxplusLog* log2 = new ngxplus::NgxplusLog();
    log2->init("test_error.log");
    common::InfoLogContext::set_log((common::AbstractLog*)log2);
    LOG(NOTICE, "test log with ngxplus_log");

    return 0;
}
