
/***********************************************
  File name		: timer.h
  Create date	: 2015-12-24 00:53
  Modified date : 2015-12-24 01:38
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "common.h"
#include "time.h"

#ifndef NGXPLUS_TIMER_H
#define NGXPLUS_TIMER_H

namespace ngxplus {

class Timer
{
public:
    virtual ~Timer();

    static std::string get_time() {
        char buf[MAX_TIME_LEN] = {'\0'};
        time_t rawtime;
        struct tm * timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        asctime_r(timeinfo, buf);

        buf[strlen(buf) - 1] = '\0';
        return std::string(buf);
    }

private:
    Timer() {}

private:
    const static uint32_t MAX_TIME_LEN = 50;
};

}

#endif
