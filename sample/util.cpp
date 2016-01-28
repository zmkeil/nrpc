
/***********************************************
  File name		: util.cpp
  Create date	: 2016-01-29 02:43
  Modified date	: 2016-01-29 02:43
  Author		: zmkeil
  Express : 
  
 **********************************************/

#include <comlog/info_log_context.h>
#include <ngxplus_log.h>
#include <ngxplus_timer.h>

namespace sample {

bool server_side_config_log()
{
    // use ngxplus_log
    ngxplus::NgxplusLog* log = new ngxplus::NgxplusLog();
    log->set_level(WARN);
    // with default path logs/error.log
    // and time_handler ngxplus::asctime
    log->init("error.log");

    common::InfoLogContext::set_log(log);
    return true;
}

bool client_side_config_log()
{
    // use default stderr_log
    // with DEBUG level and time_handler &common::asctime
    return true;
}

}
