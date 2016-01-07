#!/bin/bash

NGX_ROOT=/home/zmkeil/nginx/tengine/
tmp_dir=`dirname $0`
ROOT=`cd ${tmp_dir} && pwd`

echo "get libngx.a****************************************"
echo "cd ${NGX_ROOT} && cp $ROOT/mk_libngx.sh ./ && ./mk_libngx.sh"
cd ${NGX_ROOT} && cp $ROOT/mk_libngx.sh ./ && ./mk_libngx.sh

if [ $? -ne 0 ]; then
    echo "MAKE libngx.a error"
    exit -1;
fi

echo ""
echo "copy result*****************************************"
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

