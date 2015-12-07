
/***********************************************
  File name		: protocol.h
  Create date	: 2015-12-02 23:47
  Modified date : 2015-12-03 00:24
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

namespace nrpc
{

// A set interface of a protocol
struct Protocol {
	typedef ParseResult (*Parse)(base::IOBuf* source, Socket *socket, bool read_eof, const void *arg);
	Parse parse;

	typedef bool (*SerializeRequest)(
			base::IOBuf* request_buf,
			Controller* cntl,
			const google::protobuf::Message* request);
	SerializeRequest serialize_request;

	typedef int (*PackRequest)(
			base::IOBuf* msg, uint64_t correlation_id,
			const google::protobuf::MethodDescriptor* method,
			Controller* controller,
			const base::IOBuf& request_buf,
			const Authenticator* auth);
	PackRequest pack_request;

	typedef void (*ProcessRequest)(InputMessageBase* msg);
	ProcessRequest process_request;

	typedef void (*ProcessResponse)(InputMessageBase* msg);
	ProcessResponse process_response;

	const char* name;
}

// Interfaces of std policy
// Parse binary format of baidu-rpc
ParseResult ParseRpcMessage(base::IOBuf* source, Socket *socket, bool read_eof, const void *arg);

// Actions to a (client) request in baidu-rpc format.
void ProcessRpcRequest(InputMessageBase* msg);

// Actions to a (server) response in baidu-rpc format.
void ProcessRpcResponse(InputMessageBase* msg);

// Verify authentication information in baidu-rpc format
bool VerifyRpcRequest(const InputMessageBase* msg);

// Pack `request' to `method' into `buf'.
int PackRpcRequest(base::IOBuf* buf, uint64_t correlation_id,
		const google::protobuf::MethodDescriptor* method,
		Controller* controller,
		const base::IOBuf& request,
		const Authenticator* auth);

}
