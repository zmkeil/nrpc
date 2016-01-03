
/***********************************************
  File name		: timer.h
  Create date	: 2015-12-24 00:53
  Modified date : 2015-12-24 01:38
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

extern "C" {
#include <nginx.h>
}
#include "common.h"
#include "time.h"

#ifndef NGXPLUS_TIMER_H
#define NGXPLUS_TIMER_H

namespace ngxplus {

class Timer
{
public:
    virtual ~Timer();

    static long rawtime() {
        time_t rawtime;
        if (!NGX_PREINIT_FLAG) {
            time(&rawtime);
        } else {
            rawtime = ngx_time();
        }
        return rawtime;
    }

    static char* asctime() {
        char* result;
        time_t raw;
        struct tm * timeinfo;

        raw = (time_t)rawtime();
        timeinfo = ::localtime(&raw);
        // result point to a static mem
        result = ::asctime(timeinfo);
        result[strlen(result)-1] = '\0';
        return result;
    }

private:
    Timer() {}

private:
    const static uint32_t MAX_TIME_LEN = 50;
};

}

#endif
