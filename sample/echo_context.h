#ifndef SAMPLE_ECHO_CONTEXT_H
#define SAMPLE_ECHO_CONTEXT_H

#include <algorithm>
#include "service_context.h"
#include "service_context_log.h"

class EchoContext : public nrpc::ServiceContext
{
public:
    // 1.for session local context data
    EchoContext(std::string* s) : _comments(s), _delimiter("^,")
    {
        _svec.reserve(20);
    }
    virtual ~EchoContext()
    {
    }

    std::string* comments()
    {
        return _comments;
    }

    // 2.for service context log
    void set_session_field(const std::string key)
    {
        _svec.push_back(key);
    }

    void build_log(std::string* log)
    {
        std::for_each(_svec.begin(), _svec.end(),
            [&log, this] (std::string& key) {
                (*log) += key;
                (*log) += _delimiter;
            }
        );
    }

private:
    std::string* _comments;
    std::vector<std::string> _svec;
    std::string _delimiter;
};

class EchoContextFactory : public nrpc::ServiceContextFactory
{
public:
    EchoContextFactory(std::string* s) : _comments(s)
    {
    }

    nrpc::ServiceContext* create_context() {
        return new EchoContext(_comments);
    }

private:
    // you can put any ptr here
    std::string* _comments;
};

#endif
