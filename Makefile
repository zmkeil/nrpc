#COMAKE2 edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m),x86_64)
CC=gcc
CXX=g++
CXXFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC \
  -std=c++11
CFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CPPFLAGS=-g \
  -O2 \
  -pipe \
  -W \
  -Wall \
  -fPIC \
  -Wno-deprecated \
  -D__const__=
#  -Werror 
INCPATH=-I. \
  -I./include \
  -I../nginx/include/objs/ \
  -I../nginx/include/core/ \
  -I../nginx/include/event/ \
  -I../nginx/include/event/modules/ \
  -I../nginx/include/os/unix/ \
  -I../nginx/include/proc/ \
  -I../../common/

objs=../common/libcommon.a \
	 ./nginx/lib/libngx.a  \
	 ./ngxplus/libngxplus.a \
	 ./src/libnrpc.a


.PHONY:all
all:install test sample
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

.PHONY:clean
clean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	make -C ngxplus clean
	make -C src clean
	make -C test clean
	make -C sample clean

.PHONY:love
love:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlove[0m']"
	@echo "make love done"

./nginx/lib/libngx.a:
	@echo "[[1;32;40mlibngx.a[0m] not build yet, please do as follows"
	@echo "	cd nginx"
	@echo "	./ngxlib_install /your/path/to/nginx/"
	@exit -1

../common/libcommon.a:
	@echo "[[1;32;40mlibcommon.a[0m] not build yet, please do as follows"
	@echo "cd ../common"
	@echo "make"
	@exit -1

./ngxplus/libngxplus.a:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlibngxplus.a[0m']"
	make -C ngxplus

./src/libnrpc.a:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlibnrpc.a[0m']"
	make -C src

INCS=src/channel.h \
    src/controller.h \
    src/server.h \
    src/service_context.h \
    src/service_set.h \
	src/policy/protocol.h \
	ngxplus/ngxplus_timer.h \
	ngxplus/ngxplus_log.h \
	ngxplus/ngxplus_iobuf.h \
	ngxplus/ngxplus_open_file.h

LIBS=libngxrpc.a \
	 libngxrpc.so

libngxrpc.so:${objs}
	@echo "[[1;32;40mbuild libngxrpc.so[0m]"
	$(CXX) -shared -fPIC -Wl,--whole-archive $(objs) -Wl,--no-whole-archive  -Wl,-soname,libngxrpc.so -o libngxrpc.so

libngxrpc.a:${objs}
	@echo "[[1;32;40mbuild libngxrpc.a[0m]"
	for SF in ${objs}; \
	do \
		ar -x $$SF; \
	done
	ar cr libngxrpc.a *.o
	ranlib libngxrpc.a
	rm *.o

install:${LIBS} ${INCS}
	mkdir -p output/lib
	mkdir -p output/include
	mv libngxrpc.a libngxrpc.so output/lib/
	cp ${INCS} output/include

test:install
	@echo "[[1;32;40mbuild test[0m]"
	make -C test

sample:install
	@echo "[[1;32;40mbuild sample[0m]"
	make -C sample

endif #ifeq ($(shell uname -m),x86_64)


