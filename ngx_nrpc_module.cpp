/*
 * this is a nginx module, to add service to ngx_event.
 * the usual procedure as:
 *     1.service_set = server.push_service_set("host:port")
 *     2.service_set.add_service(service)
 *     3.ngx_start-->ngx_nrpc_module.nrpc_cmd
 */

#include <iostream>
extern "C" {
#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_string.h>
}
#include "ngx_nrpc_module.h"

/*
 * for ngx_nrpc_module init
 */
struct ngx_nrpc_listen_s {
    nrpc::ServiceAddress service_address;
    nrpc::ServiceSet *service_set;
};
typedef struct ngx_nrpc_listen_s ngx_nrpc_listen_t;

const static ngx_uint_t NRPC_MAX_LISTENS = 10;
static ngx_uint_t ngx_nrpc_listen_count = 0;
static ngx_nrpc_listen_t nrpc_listens[NRPC_MAX_LISTENS];

namespace nrpc {
int ngx_nrpc_clear_listen()
{
    ngx_memzero(nrpc_listens, NRPC_MAX_LISTENS * sizeof(ngx_nrpc_listen_t));
    return 0;
}

int ngx_nrpc_add_listen(const std::string& str_address/*/host:port:bind:wildcard:so_keepalive*/,
                nrpc::ServiceSet* service_set)
{
    ngx_nrpc_listen_t* nrpc_listen;
    ServiceAddress* service_address;

    if (ngx_nrpc_listen_count >= NRPC_MAX_LISTENS) {
        return -1;
    }
    nrpc_listen = &nrpc_listens[ngx_nrpc_listen_count++];
    nrpc_listen->service_set = service_set;

    // init service_address with str_address
    // now just the host:port
    service_address = &nrpc_listen->service_address;
    ngx_memcpy(service_address->address, str_address.c_str(), str_address.size());
    /*cmp with all the above address, return ERROR if same*/
    service_set->set_address(service_address);
    return 0;
}
}


/*
 * for ngx_nrpc_module cycle
 */
static char* ngx_nrpc_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t  ngx_nrpc_commands[] = {

    { ngx_string("nrpc"),
      NGX_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
      ngx_nrpc_block,
      0,
      0,
      NULL },

      ngx_null_command
};

static ngx_core_module_t  ngx_nrpc_module_ctx = {
    ngx_string("nrpc"),
    NULL,
    NULL
};

ngx_module_t  ngx_nrpc_module = {
    NGX_MODULE_V1,
    &ngx_nrpc_module_ctx,                  /*/ module context */
    ngx_nrpc_commands,                     /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static char *ngx_nrpc_block(ngx_conf_t *cf, ngx_command_t* /*cmd*/, void* /*conf*/)
{
    char *rv;
    ngx_uint_t             i;
    ngx_conf_t                   pcf;
    ngx_listening_t       *ls;
    ngx_str_t               url;
    ngx_url_t                   u;

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "in nrpc block");

    // TODO: parse inside the nrpc{} block
    // Now nothing in nrpc{}, this is just needed by BLOCK_DIRECTIVE
    pcf = *cf;
    rv = ngx_conf_parse(cf, NULL);
    if (rv != NGX_CONF_OK) {
        *cf = pcf;
        return rv;
    }
    *cf = pcf;

    // add listen into ngx_core_event
    for (i = 0; i < ngx_nrpc_listen_count; i++) {
        ngx_memzero(&u, sizeof(ngx_url_t));
        url.data = nrpc_listens[i].service_address.address;
        url.len = strlen((char*)nrpc_listens[i].service_address.address);
        u.url = url;
        u.listen = 1;
        if (ngx_parse_url(cf->pool, &u) != NGX_OK) {
            if (u.err) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                        "%s in \"%V\" of the \"listen\" directive",
                        u.err, &u.url);
            }
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "add listen port is: %d", u.port);

        ls = ngx_create_listening(cf, (struct sockaddr *)u.sockaddr, u.socklen);
        if (ls == NULL) {
            return (char*)NGX_CONF_ERROR;
        }

        ls->addr_ntop = 1;
        ls->handler = ngx_nrpc_init_connection;
        ls->pool_size = 256;

        // TODO: error_log directive 
        ls->logp = &cf->cycle->new_log;
        ls->log.data = &ls->addr_text;
        ls->log.handler = ngx_accept_log_error;

        // other options for listen_socket
        ls->keepalive = nrpc_listens[i].service_address.so_keepalive;

        // TODO: ls->servers
        ls->servers = &nrpc_listens[i].service_set;
    }

    return (char*)NGX_CONF_OK;
}
