
/***********************************************
  File name		: timer.cpp
  Create date	: 2016-01-19 17:56
  Modified date : 2016-01-19 17:59
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

extern "C" {
#include <nginx.h>
}
#include "common.h"
#include "time.h"
#include "timer.h"

namespace ngxplus {

long rawtime()
{
	time_t rawtime;
	if (!NGX_PREINIT_FLAG) {
		time(&rawtime);
	} else {
		rawtime = ngx_time();
	}
	return rawtime;
}

char* asctime()
{
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

// we can use ngx_cached_time or gettimeofday, gettimeofday is
// more accurate but more costed
// ngx_cached_time must be used after ngx_core_start
long rawtime_us(bool use_cache)
{
	if (use_cache) {
		return ngx_cached_time->sec * 1000 * 1000 +
			ngx_cached_time->msec * 1000 +
			ngx_cached_time->usec;
	}
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec - 1418118000) * 1000 * 1000 +
		tv.tv_usec;
}

long rawtime_us()
{
	return rawtime_us(false);
}

}
