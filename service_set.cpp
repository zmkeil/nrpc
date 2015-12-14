
#include "service_set.h"

namespace nrpc {

ServiceSet::ServiceSet(Server* server) : _server(server)
{
}

int ServiceSet::set_address(ServiceAddress* service_address)
{
    _service_address = service_address;
    return 0;
}

int ServiceSet::add_service(google::protobuf::Service* service)
{
	if (!service) {
	    return -1;
	}
    const google::protobuf::ServiceDescriptor* sd = service->GetDescriptor();
	if (_service_map.find(sd->name()) == _service_map.end()) {
	    return -1;
	}
	if (sd->method_count()) {
	    return -1;
	}

	// add _method_map
	for (int i = 0; i < sd->method_count(); ++i) {
	    const google::protobuf::MethodDescriptor* md = sd->method(i);
		const MethodProperty mp = {service, md};
		_method_map[md->full_name()] = mp;
	}

	// add _service_map
	const ServiceProperty sp = {service};
	_service_map[sd->name()] = sp;

	return 0;
}

int ServiceSet::remove_service(google::protobuf::Service* service)
{
    // remove all methods from _method_map
	
	// remove service from _service_map

	return 0;
}

google::protobuf::Service* ServiceSet::find_service_by_name(const std::string& name) const
{
	ServiceMap::const_iterator it = _service_map.find(name);
	if (it != _service_map.end()) {
	    return it->second.service;
	}
	return NULL;
}

const MethodProperty* ServiceSet::find_method_property_by_full_name(const std::string& full_name) const
{
	MethodMap::const_iterator it = _method_map.find(full_name);
	if (it != _method_map.end()) {
	    return &(it->second);
	}
	return NULL;
}

}
