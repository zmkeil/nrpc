
extern "C" {
#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_string.h>
}

struct ngx_nrpc_listen_s {
    u_char                  str_address[50];
    unsigned                so_keepalive:2;
    unsigned                bind:1;
    unsigned                wildcard:1;
};
typedef struct ngx_nrpc_listen_s ngx_nrpc_listen_t;

struct ngx_nrpc_port_s {
    /* ngx_nrpc_in_addr_t or ngx_nrpc_in6_addr_t */
    void                   *addrs;
    ngx_uint_t              naddrs;
};
typedef struct ngx_nrpc_port_s ngx_nrpc_port_t;

ngx_nrpc_listen_t nrpc_listens[10];
ngx_uint_t ngx_nrpc_listen_nelt = 0;

void ngx_add_nrpc_listen(u_char* address)
{
    ngx_nrpc_listen_t *nrpc_ls = &nrpc_listens[ngx_nrpc_listen_nelt++];
    ngx_memzero(nrpc_ls->str_address, 50);
    ngx_memcpy(nrpc_ls->str_address, address, ngx_strlen(address));
    //nrpc_ls->str_address[ngx_strlen(address)] = '\0';
}


static char *ngx_nrpc_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void ngx_nrpc_init_connection(ngx_connection_t *c);

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
    &ngx_nrpc_module_ctx,                  /* module context */
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

static char *ngx_nrpc_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char *rv;
    ngx_uint_t             i;
    ngx_conf_t                   pcf;
    ngx_listening_t       *ls;
    ngx_nrpc_port_t       *mport;
    ngx_url_t                   u;
    u_char  *address = (u_char*)"8833;";

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "in nrpc block");

     /* parse inside the socks{} block */
    pcf = *cf;
    //cf->ctx = ctx;

    //cf->module_type = NGX_HTTP_MODULE;
    //cf->cmd_type = NGX_HTTP_MAIN_CONF;
    rv = ngx_conf_parse(cf, NULL);

    if (rv != NGX_CONF_OK) {
        *cf = pcf;
        return rv;
    }
    *cf = pcf;
       
    for (i = 0; i < ngx_nrpc_listen_nelt; i++) {
        ngx_memzero(&u, sizeof(ngx_url_t));
        //ngx_str_t url = ngx_string(nrpc_listens[i].str_address/*ip:port*/);
        ngx_str_t url = ngx_string("127.0.0.1:8833");
        u.url = url;
        //u.url.data = (u_char*)"8833";
        //u.url.len = sizeof("8833");
        u.listen = 1;
        if (ngx_parse_url(cf->pool, &u) != NGX_OK) {
        //    if (u.err) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                        "%s in \"%V\" of the \"listen\" directive",
                        u.err, &u.url);
         //   }
        }
        /*cmp with all the above u, return ERROR if same*/

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                "port is: %u",((struct sockaddr_in*)u.sockaddr)->sin_port);

        ls = ngx_create_listening(cf, (struct sockaddr *)u.sockaddr, u.socklen);
        if (ls == NULL) {
            return (char*)NGX_CONF_ERROR;
        }

        ls->addr_ntop = 1;
        ls->handler = ngx_nrpc_init_connection;
        ls->pool_size = 256;

        /* TODO: error_log directive */
        ls->logp = &cf->cycle->new_log;
        ls->log.data = &ls->addr_text;
        ls->log.handler = ngx_accept_log_error;

        /*other options*/
//        ls->keepalive = nrpc_ls.so_keepalive;
//#if (NGX_HAVE_KEEPALIVE_TUNABLE)
//        ls->keepidle = nrpc_ls.tcp_keepidle;
//        ls->keepintvl = nrpc_ls.tcp_keepintvl;
//        ls->keepcnt = nrpc_ls.tcp_keepcnt;
//#endif
//
//#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
//        ls->ipv6only = nrpc_ls.ipv6only;
//#endif

/*        mport = (ngx_nrpc_port_t*)ngx_palloc(cf->pool, sizeof(ngx_nrpc_port_t));
        if (mport == NULL) {
            return (char*)NGX_CONF_ERROR;
        }

        ls->servers = mport;

        if (i == last - 1) {
            mport->naddrs = last;

        } else {
            mport->naddrs = 1;
            i = 0;
        }

        switch (ls->sockaddr->sa_family) {
#if (NGX_HAVE_INET6)
            case AF_INET6:
                if (ngx_nrpc_add_addrs6(cf, mport, addr) != NGX_OK) {
                    return NGX_CONF_ERROR;
                }
                break;
#endif
                if (ngx_nrpc_add_addrs(cf, mport, addr) != NGX_OK) {
                    return NGX_CONF_ERROR;
                }
                break;
        }

        addr++;
        last--;
*/    
    return (char*)NGX_CONF_OK;
    }
}

static void
ngx_nrpc_init_connection(ngx_connection_t *c)
{
    printf("init connection");
}

int main(int argc, char** argv)
{
    ngx_nrpc_listen_t *listen;
    u_char* address = (u_char*)"127.0.0.1:8833";

    // add nrpc module, in server.construct
    ngx_extern_modules[0] = &ngx_nrpc_module;
    ngx_extern_module_names[0] = (u_char*)"nrpc";

    // add nrpc listens, in server.add_service
    ngx_add_nrpc_listen(address);

    // start ngx_event, in server.start
    ngx_start(argc, argv);

    return 0;
}

