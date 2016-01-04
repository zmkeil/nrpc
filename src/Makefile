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
  -I./policy \
  -I../nginx/include/objs/ \
  -I../nginx/include/core/ \
  -I../nginx/include/event/ \
  -I../nginx/include/event/modules/ \
  -I../nginx/include/os/unix/ \
  -I../nginx/include/proc/ \
  -I../ngxplus \
  -I../../common

PROTO=nrpc_meta

objs=ngx_nrpc_module.o \
	 ngx_nrpc_handler.o \
	 controller.o \
	 server.o \
	 service_set.o \
	 policy/proto/nrpc_meta.pb.o \
	 policy/protocol.o \
	 policy/serialize_rpc.o \
	 policy/http_rpc.o

.PHONY:all
all:pb_c libnrpc.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

.PHONY:clean
clean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	rm -rf $(objs)
	rm -rf libnrpc.a
	rm -f policy/proto/*.pb.h
	rm -f policy/proto/*.pb.cc

.PHONY:love
love:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlove[0m']"
	@echo "make love done"

pb_c:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mpb_c[0m']"
	@cd policy/proto && for pb in $(PROTO); \
	do \
		if [ -f $$pb.pb.cc ]; then \
			timeold=`stat $$pb.pb.cc | grep -i Modify`; \
			timenew=`stat $$pb.proto | grep -i Modify`; \
			if [ timenew -lt timeold ]; then \
				echo "protoc --cpp_out=./ $$pb.proto"; \
				( protoc --cpp_out=./ $$pb.proto ) || exit 1; \
			fi \
		else \
			echo "protoc --cpp_out=./ $$pb.proto"; \
			( protoc --cpp_out=./ $$pb.proto ) || exit 1; \
		fi \
	done && cd ../

libnrpc.a:$(objs)
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlibnrpc.a[0m']"
	ar rcs libnrpc.a $(objs)

#$(objs): %.o: %.cpp
#	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
#	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o $@ $<

# obscure rules
$(filter %.pb.o,$(objs)): %.o: %.cc
	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(filter-out %.pb.o,$(filter %.o,$(objs))): %.o : %.cpp
	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

endif #ifeq ($(shell uname -m),x86_64)
