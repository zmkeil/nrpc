#!/bin/bash

./configure --without-select_module --without-poll_module --without-procs --without-syslog --without-dso --without-http --without-pcre


 # sed -i 's/^LINK/AR = ar -crv\nLINK/' ./Makefile
 #
 # sed -i 's/objs\/nginx:.*\n/objs\/libngx.a:/' ./Makefile
 # sed -i 's/\t$(LINK).*/\t$(AR) objs\/libngx.a \\\n/' ./Makefile
 # sed -i 's/.*objs\/src\/core\/nginx.o.*\n//' ./Makefile
 # sed -i 's/.*-lpthread.*//' ./Makefile
 # sed -i 's/ngx_modules.o \\/ngx_module.o/' ./Makefile
