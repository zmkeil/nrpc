
/***********************************************
  File name		: protocol.cpp
  Create date	: 2015-12-31 00:04
  Modified date : 2015-12-31 02:45
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include "protocol.h"

namespace nrpc {

extern Protocol default_protocol;
extern Protocol http_protocol;

Protocol* g_rpc_protocols[5] {
    &default_protocol,
    &http_protocol
};

}
