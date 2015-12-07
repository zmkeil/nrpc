#COMAKE2 edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m),x86_64)
CC=gcc
CXX=g++
CXXFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
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
  -I./ \
  -I../ \
  -I/home/zmkeil/nginx/tengine/objs/ \
  -I/home/zmkeil/nginx/tengine/src/core/ \
  -I/home/zmkeil/nginx/tengine/src/event/ \
  -I/home/zmkeil/nginx/tengine/src/event/modules/ \
  -I/home/zmkeil/nginx/tengine/src/os/unix/ \
  -I/home/zmkeil/nginx/tengine/src/proc/
LIBPATH=-L. \
  -L/usr/local/lib \
  -L/home/zmkeil/nginx/tengine/objs/
LDFLAGS=-lngx -lprotobuf -lpthread -lssl -lcrypto -ldl


# ##########
# files
# ##########

PROTO=echo.proto

objs=test_service.o \
	 echo.pb.o \
	 test_ngx.o

testbin=test_service test_ngx

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
	@for pb in $(PROTO); \
		do \
		echo "protoc --cpp_out=./ $$pb"; \
		( protoc --cpp_out=./ $$pb ) || exit 1; \
		done

# UT
test_service:test_service.o echo.pb.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o test_service $(LDFLAGS)

test_ngx:test_ngx.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o test_ngx $(LDFLAGS)

# obscure rules
$(filter %.pb.o,$(objs)): %.o: %.cc
	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(filter-out %.pb.o,$(filter %.o,$(objs))): %.o : %.cpp
	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

endif #ifeq ($(shell uname -m),x86_64)


