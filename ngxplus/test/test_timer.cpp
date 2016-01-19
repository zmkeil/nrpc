
/***********************************************
  File name		: test_timer.cpp
  Create date	: 2015-12-24 01:06
  Modified date : 2015-12-24 01:33
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "info_log_context.h"
#include "timer.h"

int main()
{
    LOG(NOTICE, "time is: %s", ngxplus::asctime());

    LOG(NOTICE, "rawtime is: %ld", ngxplus::rawtime());
    return 0;
}
