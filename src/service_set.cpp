
#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <cxxabi.h>
#include <google/protobuf/message.h>
#include <string_printf.h>
#include <comlog/info_log_context.h>
#include "service_set.h"

namespace nrpc {
ServiceSet::ServiceSet()
{
}

ServiceSet::ServiceSet(Server* server) : _server(server)
{
}

void ServiceSet::set_address(ServiceAddress* service_address)
{
    _service_address = service_address;
}

bool ServiceSet::add_service(google::protobuf::Service* service)
{
	if (!service) {
        LOG(WARN, "add service NULL");
	    return false;
	}
    const google::protobuf::ServiceDescriptor* sd = service->GetDescriptor();
	if (_service_map.find(sd->name()) != _service_map.end()) {
        LOG(WARN, "the service \"%s\" already existed", sd->name().c_str());
	    return false;
	}
	if (!sd->method_count()) {
        LOG(WARN, "the service \"%s\" has no method", sd->name().c_str());
	    return false;
	}

	// add _method_map
	for (int i = 0; i < sd->method_count(); ++i) {
	    const google::protobuf::MethodDescriptor* md = sd->method(i);
		const MethodProperty mp = {service, md};
        std::string full_name = sd->name() + "_" + md->name();
		//_method_map[full_name] = mp;
        _method_map.insert(std::make_pair(full_name, mp));
	}

	// add _service_map
	const ServiceProperty sp = {service};
	_service_map[sd->name()] = sp;

	return true;
}

bool ServiceSet::remove_service(google::protobuf::Service* service)
{
    // TODO: remove_service
    (void) service;
    // remove all methods from _method_map

	// remove service from _service_map

	return true;
}

const ServiceProperty* ServiceSet::find_service_by_name(const std::string& service_name)
{
	ServiceMap::iterator it = _service_map.find(service_name);
    return (it != _service_map.end()) ? &(it->second) : NULL;
}

const MethodProperty* ServiceSet::find_method_property_by_full_name(const std::string& method_full_name)
{
	MethodMap::iterator it = _method_map.find(method_full_name);
    return (it != _method_map.end()) ? &(it->second) : NULL;
}

void ServiceSet::dump(std::string *message)
{
    common::string_appendf(message, "dump ServiceSet in \"%s\"\nHas [%ld] services:\n",
            (char*)_service_address->address, _service_map.size());
    std::for_each(_service_map.begin(), _service_map.end(),
        [&message] (std::pair<std::string, ServiceProperty>service_item) {
            google::protobuf::Service* service = service_item.second.service;
            const google::protobuf::ServiceDescriptor* sd = service->GetDescriptor();
            common::string_appendf(message, "  %s with [%d] methods:\n", sd->name().c_str(), sd->method_count());
            for (int i = 0; i < sd->method_count(); ++i) {
                const google::protobuf::MethodDescriptor* md = sd->method(i);
                const google::protobuf::Message& default_request_instance = service->GetRequestPrototype(md);
                const google::protobuf::Message& default_response_instance = service->GetResponsePrototype(md);
                common::string_appendf(message, "    %s (%s) returns (%s)\n", md->name().c_str(),
                    abi::__cxa_demangle(typeid(default_request_instance).name(), NULL, NULL, NULL),
                    abi::__cxa_demangle(typeid(default_response_instance).name(), NULL, NULL, NULL));
            }
        }
    );
}

Server* ServiceSet::server()
{
    return _server;
}

}
