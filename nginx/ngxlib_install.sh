#!/bin/bash

NGX_ROOT=/home/zmkeil/nginx/tengine/
tmp_dir=`dirname $0`
ROOT=`cd ${tmp_dir} && pwd`

echo "======="
echo "cd ${NGX_ROOT} && cp $ROOT/mk_libngx.sh ./ && ./mk_libngx.sh"
cd ${NGX_ROOT} && cp $ROOT/mk_libngx.sh ./ && ./mk_libngx.sh

echo ""
echo "======="
echo "cd ${NGX_ROOT}src/core"
cd ${NGX_ROOT}src/core && if [ ! -f nginx.c.org -o ! -f nginx.h.org ]; then \
    echo "copy nginx.c.h to nginx.c.h.org"; \
    cp nginx.c nginx.c.org && cp nginx.h nginx.h.org; \
else \
    echo "nginx.c.h.org already exist"; \
fi
echo "cp $ROOT/nginx.c ./ && cp $ROOT/nginx.h ./"
cp $ROOT/nginx.c ./ && cp $ROOT/nginx.h ./

echo ""
echo "======="
echo "cd ${NGX_ROOT}/objs"
cd ${NGX_ROOT}/objs
echo "modify objs/Makefile and objs/ngx_modules.c"
sed -i 's/-Werror //g' ./Makefile
sed -i 's/objs\/nginx:	objs/objs\/libngx.a:	objs/g' ./Makefile
sed -i 's/LINK =.*/AR = ar -rcs/g' ./Makefile
sed -i 's/LINK/AR/g' ./Makefile
sed -i 's/-o objs\/nginx/objs\/libngx.a/g' ./Makefile
sed -i 's/	objs\/ngx_modules\.o \\/	objs\/ngx_modules.o/g' ./Makefile
sed -i 's/-lpthread.*//g' ./Makefile
sed -i 's/NULL/NULL,\n    NULL/g' ./ngx_modules.c

echo ""
echo "======="
echo "cd ${NGX_ROOT} && make"
cd ${NGX_ROOT} && make && \

echo ""
echo "======="
echo "cd ${ROOT}"
echo "cp lib include ${ROOT}"
cd ${ROOT} && mkdir -p include/core \
&& mkdir -p include/event \
&& mkdir -p include/event/modules \
&& mkdir -p include/os/unix \
&& mkdir -p include/proc \
&& mkdir -p include/objs \
&& mkdir -p lib
cp ${NGX_ROOT}/src/core/*.h ./include/core/
cp ${NGX_ROOT}/src/event/*.h ./include/event/
cp ${NGX_ROOT}/src/event/modules/*.h ./include/event/modules/
cp ${NGX_ROOT}/src/os/unix/*.h ./include/os/unix/
cp ${NGX_ROOT}/src/proc/*.h ./include/proc/
cp ${NGX_ROOT}/objs/*.h ./include/objs/
cp ${NGX_ROOT}/objs/libngx.a ./lib/

echo ""
echo "======="
echo "cd ${NGX_ROOT}/src/core"
echo "cp nginx.c.h.org nginx.c.h"
cd ${NGX_ROOT}/src/core \
&& cp nginx.c.org nginx.c \
&& cp nginx.h.org nginx.h
echo "rm ${NGX_ROOT}/objs/Makefile"
rm -rf ${NGX_ROOT}/objs/Makefile
