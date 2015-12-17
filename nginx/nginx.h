
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGINX_H_INCLUDED_
#define _NGINX_H_INCLUDED_

#include <ngx_core.h>

#define nginx_version      1006002
#define NGINX_VERSION      "1.6.2"
#define NGINX_VER          "nginx/" NGINX_VERSION

#define TENGINE            "Tengine"
#define tengine_version    2001000
#define TENGINE_VERSION    "2.1.0"
#define TENGINE_VER        TENGINE "/" TENGINE_VERSION

#define NGINX_VAR          "NGINX"
#define NGX_OLDPID_EXT     ".oldbin"

int ngx_pre_init(ngx_log_t **log);
int ngx_start();
extern ngx_module_t* ngx_extern_modules[];
extern u_char* ngx_extern_module_names[];

#endif /* _NGINX_H_INCLUDED_ */
