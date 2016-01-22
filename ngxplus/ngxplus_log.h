
/***********************************************
  File name		: log.h
  Create date	: 2015-12-17 23:12
  Modified date : 2016-01-19 17:43
  Author		: zmkeil, alibaba.inc
  Express : 
    Usually, log module is first inited,
    if error, write into stderr
  
 **********************************************/
#ifndef NGXPLUS_LOG_H
#define NGXPLUS_LOG_H

#include <string>
#include <comlog/abstract_log.h>

namespace ngxplus {

class NgxplusLog : common::AbstractLog
{
public:
    NgxplusLog() : AbstractLog() {}
    NgxplusLog(int level) : AbstractLog(level) {}

    bool init();
    bool init(const std::string& name);

    void comlog_write_core(int level, const char* fmt, va_list args);

private:
    int _fd;

private:
    const static char* LOG_PATH;
    const static int MAX_ERROR_STR = 500;
};

} // ngxplus
#endif
