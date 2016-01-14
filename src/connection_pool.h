
/***********************************************
  File name		: connection_pool.h
  Create date	: 2016-01-14 22:32
  Modified date	: 2016-01-14 22:32
  Author		: zmkeil
  Express : 
  
 **********************************************/
#ifndef NRPC_CONNECTION_POOL_H
#define NRPC_CONNECTION_POOL_H

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
}
#include <map>
#include <vector>

namespace nrpc {

struct ConnectionInfo
{
	ConnectionInfo() : thid(0), is_idle(true), reuse_time(0) {}

    pthread_t thid;
    bool is_idle;
    int reuse_time;
};

typedef std::map<int, ConnectionInfo> ConnectionMap;

class ConnectionPool {
public:
    ConnectionPool();
    virtual ~ConnectionPool();
    bool init(const char* server_addr, int port, int connect_timeout);

public:
/*
 * these two function deal with network, may time-cost
 * without sync lock
 */
    // this function without sync lock. it is called:
    //   1.when channel.init with the flag_reserve_connection(NOT implemented yet).
    //   2.if reuse_connection() return -1, then new_connection()
	// why no sync_lock
	//   build a new connection is a time-cost operate, and this function doesn't 
	//   operate the _connection_pool
    int new_connection();
	// really close the connection from the _close_connection_queue, just use push_back()
	// and pop_back(), the thread-safe is guaranteed by STL:vector
    // if flag close_all is true, close all connections in _connection_pool, and this must be called after channel_join()
	void close_connection(bool close_all);
    void close_connection();

/*
 * these three function deal with _connection_pool
 * must under sync lock
 */
    // the connection_pool operation
    // be called when launch a rpc_call, return idle and clear sockfd from _connection_pool.
	// this function must be called under sync lock
    int reuse_connection();
    // release a connection after the last rpc parse DONE. if new connection, push
	// it into _connection_pool, otherwise modify its state. under sync lock.
    bool release_connection(int sockfd);
    // drop a connection when: under sync lock
    // 1.any error (read/recv/parse) occurs in this connection, avoid dirty data
    // 2.server close the connection. there are two point to check if the connection is closed by server:
    //   I)check eof when recv response. it works if server.is_connection_reuse is false
    //   II)after reuse_connection. this check is not guaranteed absolute effective because the FIN from
    //   server may come just after reuse_connection, so if READ_ERROR returns, we should try again with
    //   a new_connection()
	// NOTE: this function just move the socket to the drop_connection_queue, close_connection()
	// really close these connection. Now it is hooked at finalize_client()
    bool drop_connection(int sockfd);

public:
    struct sockaddr* servaddr() {
        return (struct sockaddr*)&_servaddr;
    }
    socklen_t servaddr_len() {
        return _servaddr_len;
    }
    pthread_mutex_t* mutex() {
        return &_mutex;
    }

private:
    // the real function operate about _connection_pool
    int reuse_connection_handler();
    bool release_connection_handler(int sockfd);
    bool drop_connection_handler(int sockfd);

private:
    // Don't implement something like lb(loadbalance), just mutil-connections over one channel(the
    // same server-end ip:port).
    const char* _server_ip;
    int _server_port;
    struct sockaddr_in _servaddr;
    socklen_t _servaddr_len;

    // connect timeout
    int _connect_timeout;

    // Implement a simple connection-pool, a connection can only be used by one pthread(rpc_call)
    // in a time, all the information are recorded in _connection_pool, and be processed in pre-thread,
    // so the function reuse_connection()/release_connection()/drop_connection() must be thread-safe.
    pthread_mutex_t _mutex;
    ConnectionMap _connection_pool;
	// dropped connection, be really closed asynchronously
	std::vector<int> _close_connection_queue/*sockfd*/;

};

}
#endif
