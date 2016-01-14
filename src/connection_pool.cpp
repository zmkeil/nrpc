
/***********************************************
  File name		: connection_pool.cpp
  Create date	: 2016-01-14 22:57
  Modified date	: 2016-01-14 22:57
  Author		: zmkeil
  Express : 
  
 **********************************************/

#include <common.h>
#include "info_log_context.h"
#include "connection_pool.h"

namespace nrpc {

ConnectionPool::ConnectionPool()
{
    _mutex = PTHREAD_MUTEX_INITIALIZER;
}

ConnectionPool::~ConnectionPool()
{
    pthread_mutex_destroy(&_mutex);
}

bool ConnectionPool::init(const char* server_addr, int port, int connect_timeout)
{
    _server_ip = server_addr;
    _server_port = port;
    _servaddr_len = sizeof(_servaddr);
    if (!common::sockaddr_init(&_servaddr, _servaddr_len, server_addr, port)) {
        LOG(NGX_LOG_LEVEL_ALERT, "error ip format \"%s\"", _server_ip);
        return false;
    }

    _connect_timeout = connect_timeout;
    return true;
}

int ConnectionPool::new_connection()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd != -1) {
		if (!common::connect_with_timeout(sockfd, (struct sockaddr*)&_servaddr,
					_servaddr_len, _connect_timeout)) {
			sockfd = -1;
		}
	}

    LOG(INFO, "new connection [%d]", sockfd);
    return sockfd;
}

void ConnectionPool::close_connection(bool close_all)
{
	while (!_close_connection_queue.empty()) {
		int sockfd = _close_connection_queue.back();
        // the pop_back thread-safe is gguaranteed by STL:vector
		_close_connection_queue.pop_back();
		LOG(INFO, "close connection [%d]", sockfd);
		close(sockfd);
	}

    if (close_all) {
        // close all connection in _connection_pool
        LOG(INFO, "close all connections");
    }
}

void ConnectionPool::close_connection()
{
    close_connection(false);
}

int ConnectionPool::reuse_connection_handler()
{
    ConnectionMap::iterator it;
    for (it = _connection_pool.begin(); it != _connection_pool.end(); ++it) {
        LOG(INFO, "find old connection");
        ConnectionInfo& cinfo = it->second;
        if (cinfo.is_idle) {
            // check if server close, but this can't guarantee that the connection is
            // still establish when you send data.
            // So in client side, we should provide retry mechanism
            int sockfd = it->first;
            if (!common::is_socket_clear_and_idle(sockfd)) {
                LOG(INFO, "server already close connection");
                drop_connection_handler(sockfd);
            } else {
                cinfo.is_idle = false;
                break;
            }
        }
    }

    int sockfd = -1;
    if (it != _connection_pool.end()) {
        sockfd = it->first;
	}
    LOG(INFO, "reuse connection [%d]", sockfd);
    return sockfd;
}

int ConnectionPool::reuse_connection()
{
    auto mutex_scope = common::makeScopeGuard(
        [this] () {
            if (pthread_mutex_lock(&_mutex) != 0) {
                LOG(ALERT, "lock mutex error: %s", strerror(errno));
                pthread_exit((void*)"get mutex error");
            }
        },
        [this] () {pthread_mutex_unlock(&_mutex);}
    );
    return reuse_connection_handler();
}

bool ConnectionPool::release_connection_handler(int sockfd)
{
    // if sockfd not exists, STL:map will insert a new with default constructor
	_connection_pool[sockfd].is_idle = true;

    LOG(INFO, "release connection [%d]", sockfd);
    return true;
}

bool ConnectionPool::release_connection(int sockfd)
{
    auto mutex_scope = common::makeScopeGuard(
        [this] () {
            if (pthread_mutex_lock(&_mutex) != 0) {
                LOG(ALERT, "lock mutex error: %s", strerror(errno));
                pthread_exit((void*)"get mutex error");
            }
        },
        [this] () {pthread_mutex_unlock(&_mutex);}
    );
    return release_connection_handler(sockfd);
}

bool ConnectionPool::drop_connection_handler(int sockfd)
{
    LOG(INFO, "drop connection [%d]", sockfd);
	_close_connection_queue.push_back(sockfd);
    ConnectionMap::iterator it = _connection_pool.find(sockfd);
    if (it != _connection_pool.end()) {
        _connection_pool.erase(it);
        return true;
    }
    return false;
}

bool ConnectionPool::drop_connection(int sockfd)
{
    auto mutex_scope = common::makeScopeGuard(
        [this] () {
            if (pthread_mutex_lock(&_mutex) != 0) {
                LOG(ALERT, "lock mutex error: %s", strerror(errno));
                pthread_exit((void*)"get mutex error");
            }
        },
        [this] () {pthread_mutex_unlock(&_mutex);}
    );
    return drop_connection_handler(sockfd);
}

}
