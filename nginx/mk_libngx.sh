#!/bin/bash

function check_nu()
{
    if [ $1x == "x" ]; then
        echo "get nu error"
        exit -1
    fi
}

echo ""
echo "Configure======================================>"
echo ""

./configure --without-select_module --without-poll_module --without-procs --without-syslog --without-dso --without-http --without-pcre


echo ""
echo "Change build ctx======================================>"
echo ""

cd objs
echo "modify objs/Makefile and objs/ngx_modules.c"
sed -i 's/-Werror //g' ./Makefile
sed -i 's/objs\/nginx:	objs/objs\/libngx.a:	objs/g' ./Makefile
sed -i 's/LINK =.*/AR = ar -rcs/g' ./Makefile
sed -i 's/LINK/AR/g' ./Makefile
sed -i 's/-o objs\/nginx/objs\/libngx.a/g' ./Makefile
sed -i 's/	objs\/ngx_modules\.o \\/	objs\/ngx_modules.o/g' ./Makefile
sed -i 's/-lpthread.*//g' ./Makefile
sed -i 's/NULL/NULL,\n    NULL/g' ./ngx_modules.c
cd ..


echo ""
echo "Change nginx.c.h======================================>"
echo ""

cd src/core
if [ ! -f nginx.c.org -o ! -f nginx.h.org ]; then
    echo "copy nginx.c.h to nginx.c.h.org"
    cp nginx.c nginx.c.org && cp nginx.h nginx.h.org
else
    echo "nginx.c.h.org already exist"
fi


echo ""
echo "nginx.h=======>"
echo ""

echo "cp nginx.h.org nginx.h"
cp nginx.h.org nginx.h
echo "Modify nginx.h"
nu=`awk '{if($0 ~ /#endif/){print NR}}' ./nginx.h | tail -1`
check_nu $nu

sed -i "${nu} s/^/\n/" ./nginx.h
sed -i "${nu} s/^/int ngx_start();\n/" ./nginx.h
sed -i "${nu} s/^/extern u_char* ngx_extern_module_names[];\n/" ./nginx.h
sed -i "${nu} s/^/extern ngx_module_t* ngx_extern_modules[];\n/" ./nginx.h
sed -i "${nu} s/^/extern int NGX_PREINIT_FLAG;\n/" ./nginx.h
sed -i "${nu} s/^/#include <ngx_core.h>\n/" ./nginx.h

echo ""
echo "nginx.c======>"
echo ""

echo "cp nginx.c.org nginx.c"
cp nginx.c.org nginx.c
echo "Modify nginx.c"
nu=`awk '{if($0 ~ /main\(int/){print NR}}' ./nginx.c`
main_end_nu=`awk 'BEGIN{flag=0}                     \
{                                                   \
    if($0 ~ /main\(int/) {flag=1}                   \
    if(($0 ~ /^}/) && (flag==1)) {print NR; flag=0} \
}' ./nginx.c`
check_nu $nu
check_nu ${main_end_nu}

echo "main -> ngx_start ==>"
sed -i "${nu},${main_end_nu} s/main(.*/ngx_start()/" ./nginx.c

echo "s/argc/nrpc_argc/ && s/argv/nrpc_argv/==>"
echo "main function form ${nu} to ${main_end_nu}"
sed -i "${nu},${main_end_nu} s/argc/nrpc_argc/" ./nginx.c
sed -i "${nu},${main_end_nu} s/ argv/ nrpc_argv/" ./nginx.c

echo "add extern vars==>"
nu=$((nu-1))
sed -i "${nu} s/^/\n/" ./nginx.c
sed -i "${nu} s/^/int NGX_PREINIT_FLAG = 0;\n/" ./nginx.c
sed -i "${nu} s/^/static char *nrpc_argv[1];\n/" ./nginx.c
sed -i "${nu} s/^/static int nrpc_argc;\n/" ./nginx.c
sed -i "${nu} s/^/};\n/" ./nginx.c
sed -i "${nu} s/^/    NULL\n/" ./nginx.c
sed -i "${nu} s/^/u_char *ngx_extern_module_names[10] = {\n/" ./nginx.c
sed -i "${nu} s/^/};\n/" ./nginx.c
sed -i "${nu} s/^/    NULL\n/" ./nginx.c
sed -i "${nu} s/^/ngx_module_t *ngx_extern_modules[10] = {\n/" ./nginx.c
nu=$((nu+10+3))
sed -i "${nu} s/^/    memcpy(nrpc_argv[0], \"nrpc\", 4);\n/" ./nginx.c
sed -i "${nu} s/^/    nrpc_argv[0] = (char*)calloc(1, 10);\n/" ./nginx.c
sed -i "${nu} s/^/    nrpc_argc = 1;\n/" ./nginx.c

echo "change ngx_prefix_conf==>"
nu=`awk '{if($0 ~ /ngx_prefix/){print NR}}' ./nginx.c | head -1`
check_nu ${nu}
nu_end=$((nu+5))
sed -i "${nu},${nu_end} s/ngx_prefix/ngx_prefix = (u_char*)\"\.\/\"/" ./nginx.c
sed -i "${nu},${nu_end} s/ngx_conf_file/ngx_conf_file = (u_char*)\"conf\/nginx.conf\"/" ./nginx.c

echo "add NGX_PRE_INIT==>"
nu=`awk '{if($0 ~ /if \(ngx_process_options/) {print NR}}' ./nginx.c | head -1`
check_nu ${nu}
sed -i "${nu} s/^/\n/" ./nginx.c
sed -i "${nu} s/^/    NGX_PREINIT_FLAG = 1;\n/" ./nginx.c

echo "add add_extern_modules function==>"
nu=`awk '{if($0 ~ /ngx_max_module = 0/){print NR}}' ./nginx.c`
add_module_end_nu=`awk 'BEGIN{flag=0}                             \
{                                                           \
    if($0 ~ /ngx_max_module = 0/) {flag=1}                  \
    if(($0 ~ /^    }/) && (flag==1)) {print NR; flag=0}     \
}' ./nginx.c`
check_nu $nu
check_nu ${add_module_end_nu}
add_module_end_nu=$((add_module_end_nu+1))

sed -i "${add_module_end_nu} s/^/    }\n/" ./nginx.c
sed -i "${add_module_end_nu} s/^/        ngx_max_module++;\n/" ./nginx.c
sed -i "${add_module_end_nu} s/^/        ngx_modules[ngx_max_module]->index = ngx_max_module;\n/" ./nginx.c
sed -i "${add_module_end_nu} s/^/        ngx_module_names[ngx_max_module] = ngx_extern_module_names[i];\n/" ./nginx.c
sed -i "${add_module_end_nu} s/^/        ngx_modules[ngx_max_module] = ngx_extern_modules[i];\n/" ./nginx.c
sed -i "${add_module_end_nu} s/^/    for (i = 0; ngx_extern_modules[i]; i++) {\n/" ./nginx.c

cd ../..


echo ""
echo "MAKE======================================>"
echo ""

echo "make"
make
echo "copy nginx.c.h.org nginx.c.h"
#cd core/src
#cp nginx.c.org nginx.c
#cp nginx.h.org nginx.h
#cd ../..
echo "rm objs/Makefile"
#rm -rf objs/Makefile

if [ ! -f objs/libngx.a ]; then
    echo "ar libngx.a failed"
    exit -1
fi
echo ""
echo "get libngx.a successly==============================>"
echo ""

