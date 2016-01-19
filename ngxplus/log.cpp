
/***********************************************
  File name		: log.cpp
  Create date	: 2015-12-17 23:28
  Modified date : 2016-01-19 17:29
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>
}
#include "log.h"

namespace ngxplus {
/* 
 * common LOG
 * init(), then write()
 */

// write log in dir: ./logs/
const char* Log::LOG_PATH = "./logs/";

static const char *err_levels[] = {
    "STDERR",
    "emerg",
    "alert",
    "crit",
    "error",
    "warn",
    "notice",
    "info",
    "debug"
};

bool Log::init()
{
    _fd = ngx_stderr;
    _level = 0;
    return true;
}

bool Log::init(const std::string& log_name)
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

    _level = 0;
    return true;
}

bool Log::init(const std::string& log_name, int level)
{
    if (!init(log_name)) {
        return false;
    }
    _level = level;
    return true;
}

void Log::comlog_write(int level, const char* fmt, ...)
{
    va_list args;

    if (level <= _level) {
        va_start(args, fmt);
        comlog_write_core(level, fmt, args);
        va_end(args);
    }
}

void Log::comlog_write_core(int level, const char* fmt, va_list args)
{
    u_char errstr[MAX_ERROR_STR];
    u_char* p;
    u_char* last;

    p = errstr;
    last = errstr + MAX_ERROR_STR;

    p = ngx_slprintf(p, last, " [%s] ", err_levels[level]);

    p = ngx_vslprintf(p, last, fmt, args);

    ngx_linefeed(p);

    (void) ngx_write_fd(_fd, errstr, p - errstr);
    return;
}

} // ngxplus
