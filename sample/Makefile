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
  -std=c++11 \
  -Dprivate=public \
  -Dprotected=public
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
  -I../src/ \
  -I../src/policy/ \
  -I../src/policy/proto \
  -I../nginx/include/objs/ \
  -I../nginx/include/core/ \
  -I../nginx/include/event/ \
  -I../nginx/include/event/modules/ \
  -I../nginx/include/os/unix/ \
  -I../nginx/include/proc/ \
  -I../ngxplus/ \
  -I../../common/
LIBPATH=-L/usr/local/lib \
  -L. \
  -L../src/ \
  -L../nginx/lib/ \
  -L../ngxplus/ \
  -L../../common/
#LDFLAGS=-Xlinker "-(" -lngxplus -lngx -lnrpc -Xlinker "-)" -lprotobuf -lpthread -lssl -lcrypto -ldl
LDFLAGS=-lnrpc -lngxplus -lngx -lcommon -lprotobuf -lpthread -lssl -lcrypto -ldl


# ##########
# files
# ##########

PROTO=echo

objs=echo.pb.o \
	 server.o \
	 server_service_context.o \
	 server_connection_reuse.o \
	 server_max_concurrency.o \
	 client.o \
	 client_async.o \
	 client_sync.o \
	 client_connection_reuse.o

testbin=server \
		server_service_context \
		server_connection_reuse \
		server_max_concurrency \
        client \
		client_async \
		client_sync \
		client_connection_reuse

.PHONY:all
all:pb_c $(testbin)
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

.PHONY:clean
clean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	rm -rf $(testbin)
	rm -rf *.pb.h
	rm -rf *.pb.cc
	rm -rf $(objs)

.PHONY:love
love:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlove[0m']"
	@echo "make love done"

pb_c:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mpb_c[0m']"
	@for pb in $(PROTO); \
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
	done

# UT
server:server.o echo.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o server $(LDFLAGS)

server_service_context:server_service_context.o echo.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o server_service_context $(LDFLAGS)

server_connection_reuse:server_connection_reuse.o echo.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o server_connection_reuse $(LDFLAGS)

server_max_concurrency:server_max_concurrency.o echo.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o server_max_concurrency $(LDFLAGS)

client:client.o echo.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o client $(LDFLAGS)

client_async:client_async.o echo.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o client_async $(LDFLAGS)

client_sync:client_sync.o echo.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o client_sync $(LDFLAGS)

client_connection_reuse:client_connection_reuse.o echo.pb.o
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40m$@[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o client_connection_reuse $(LDFLAGS)

# obscure rules
$(filter %.pb.o,$(objs)): %.o: %.cc
	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(filter-out %.pb.o,$(filter %.o,$(objs))): %.o : %.cpp
	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

endif #ifeq ($(shell uname -m),x86_64)


