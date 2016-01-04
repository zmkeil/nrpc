
/***********************************************
  File name		: client.cpp
  Create date	: 2015-12-31 23:56
  Modified date : 2016-01-01 02:52
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/
extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
}
#include <iostream>
#include "controller.h"
#include "protocol.h"
#include "echo.pb.h"

class EchoServiceImpl : public nrpc::EchoService {
public:
	EchoServiceImpl() {};
	virtual ~EchoServiceImpl() {};
	virtual void Echo(google::protobuf::RpcController* cntl_base, 
			const nrpc::EchoRequest* request, 
			nrpc::EchoResponse* response, 
			google::protobuf::Closure* done) {
		std::cout << "in Echo method" << std::endl;
        std::cout << "request msg: " << request->msg() << std::endl;
		return;
	}
};

#define PROTOCOL_NUM 0
#define SERV_PORT 8899

int main(int argc, char** argv)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        std::cout << "usage: tcpcli <IPaddress>" << std::endl;
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    EchoServiceImpl service;
    // pack request to iobuf
    const google::protobuf::ServiceDescriptor* psdes = service.GetDescriptor();
	const google::protobuf::MethodDescriptor *pmdes = psdes->method(0);
    nrpc::EchoRequest req;
    req.set_msg("hello");
    ngxplus::IOBuf iobuf;
    nrpc::Protocol* protocol = nrpc::g_rpc_protocols[PROTOCOL_NUM];
    protocol->pack_request(&iobuf, pmdes, NULL, req);

    send(sockfd, iobuf.get_read_point(), iobuf.get_byte_count(), 0);
    //sleep(1);

    iobuf.release_all();
    ngxplus::IOBufAsZeroCopyOutputStream zero_out_stream(&iobuf);
    char *buf;
    int size;
    while (zero_out_stream.Next((void**)&buf, &size)) {
        if (size == 0) {
            continue;
        }
        int n = recv(sockfd, buf, size, 0);
        std::cout << n << std::endl;
        if (n < 0) {
            std::cout << "read error" << std::endl;
            return -1;
        }
        if ((n == 0) || (n < size)) {
            // parse, then determine process_response or next loop
            // here just process_response
            break;
        }
    }

    iobuf.print_payload();
    return 0;
}
