
/***********************************************
  File name		: service_set.h
  Create date	: 2015-12-02 22:04
  Modified date : 2015-12-04 00:41
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/

#include <vector>
#include <string>
#include <map>
#include <google/protobuf/service.h>                // google::protobuf::Service
#include <google/protobuf/descriptor.h>

namespace nrpc {

struct ServiceAddress {
    u_char                  address[100];
    unsigned                so_keepalive:2;
    unsigned                bind:1;
    unsigned                wildcard:1;
};

struct ServiceProperty {
	google::protobuf::Service* service;
};
typedef std::map<std::string, ServiceProperty> ServiceMap;

struct MethodProperty {
	google::protobuf::Service* service;
	const google::protobuf::MethodDescriptor* method;
};
typedef std::map<std::string, MethodProperty> MethodMap;

class ServiceSet {
public:
	ServiceSet(Server* server);
	~ServiceSet() {};

	// init address of the service_set 
	int set_address(ServiceAddress* service_address);

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
    // address of the service_set
    ServiceAddress* _service_address;
    // ref to server
    Server* _server;
};

}
