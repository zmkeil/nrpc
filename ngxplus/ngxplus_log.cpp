
/***********************************************
  File name		: log.cpp
  Create date	: 2015-12-17 23:28
  Modified date : 2016-01-19 17:29
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

extern "C" {
#include <ngx_core.h>
}
#include "ngxplus_log.h"
#include "ngxplus_timer.h"

namespace ngxplus {
/* 
 * common LOG
 * init(), then write()
 */

// write log in dir: ./logs/
const char* NgxplusLog::LOG_PATH = "./logs/";

static const char *err_levels[] = {
    "STDERR",
    "emerg",
    "alert",
    "error",
    "warn",
    "notice",
    "info",
    "debug"
};

bool NgxplusLog::init()
{
    return init("error.log");
}

bool NgxplusLog::init(const std::string& log_name)
{
    if (log_name.size() == 0) {
        ngx_log_stderr(0, "[alert] log_name is null");
        return false;
    }

    std::string file_name(LOG_PATH);
    file_name.append(log_name);
    _fd = ngx_open_file(file_name.c_str(), NGX_FILE_APPEND,
            NGX_FILE_CREATE_OR_OPEN, NGX_FILE_DEFAULT_ACCESS);
    if (_fd == NGX_INVALID_FILE) {
        ngx_log_stderr(0, "[alert] could not open error log file: \"%s\"",
                file_name.c_str());
        return false;
    }

    _time_handler = &ngxplus::asctime;

    return true;
}

void NgxplusLog::comlog_write_core(int level, const char* fmt, va_list args)
{
    u_char errstr[MAX_ERROR_STR];
    u_char* p;
    u_char* last;

    p = errstr;
    last = errstr + MAX_ERROR_STR;

    p = ngx_slprintf(p, last, " [%s] %s ", err_levels[level], _time_handler());

    p = ngx_vslprintf(p, last, fmt, args);

    ngx_linefeed(p);

    (void) ngx_write_fd(_fd, errstr, p - errstr);
    return;
}

} // ngxplus
