
/***********************************************
  File name		: timer.h
  Create date	: 2015-12-24 00:53
  Modified date : 2016-01-19 18:00
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#ifndef NGXPLUS_TIMER_H
#define NGXPLUS_TIMER_H

namespace ngxplus {

long rawtime();

char* asctime();

long rawtime_us(bool use_cache);

long rawtime_us();

}

#endif
