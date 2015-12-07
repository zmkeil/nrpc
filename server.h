
/***********************************************
  File name		: server.h
  Create date	: 2015-12-02 22:04
  Modified date : 2015-12-04 00:41
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/
#include <string>
#include <map>
#include <google/protobuf/service.h>                // google::protobuf::Service
#include <google/protobuf/descriptor.h>

namespace nrpc {

struct ServiceProperty {
	google::protobuf::Service* service;
};
typedef std::map<std::string, ServiceProperty> ServiceMap;

struct MethodProperty {
	google::protobuf::Service* service;
	const google::protobuf::MethodDescriptor* method;
};
typedef std::map<std::string, MethodProperty> MethodMap;

class Server {
public:
	Server();
	~Server();

	// Start server
	int start(/*ip:port*/);

	// Stop server
	int stop();

	// Join all processor after server.stop()
	int join();


	// Add service
	int add_service(google::protobuf::Service* service);

	// Remove service
    int remove_service(google::protobuf::Service* service);

	// Find a service by its ServiceDescriptor::name().
	google::protobuf::Service* find_service_by_name(const std::string& name) const;

	// Find methodProperty by full_name, then can access methodDescriptor and service
	const MethodProperty* find_method_property_by_full_name(const std::string& full_name) const;

private:
	// <service->name(), ServiceProperty>
	ServiceMap _service_map;
	// <method->full_name(), MethodProperty>
	MethodMap _method_map;
};

}
