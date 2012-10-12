#
# Use this makefile, we have to compile the nginx before starting compile this demo.
# We just use nginx to write demo programs.
#

TARGETS := basic_types_int \
		   basic_types_str \
		   basic_types_pool \
		   basic_types_array \
		   basic_types_queue \
           basic_types_hash

NGX_ROOT := ../../nginx-1.2.4

CC := gcc
CFLAGS := -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g
INCS := -I. \
		-I$(NGX_ROOT)/src/core \
		-I$(NGX_ROOT)/src/event \
		-I$(NGX_ROOT)/src/event/modules \
		-I$(NGX_ROOT)/src/os/unix \
		-I$(NGX_ROOT)/objs \
		-I$(NGX_ROOT)/src/http \
		-I$(NGX_ROOT)/src/http/modules
LDFLAGS :=
LIBS := -lcrypt -lpcre -lcrypto -lz

all: $(TARGETS)

valgrind: $(TARGETS)
	for prog in $(TARGETS); do \
		valgrind --tool=memcheck --leak-check=full ./$$prog; \
	done

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean valgrind

basic_types_int: basic_types_int.c
	$(CC) $(CFLAGS) $(INCS) $^ -o $@ $(LDFLAGS) $(LIBS)

basic_types_str: basic_types_str.c
	$(CC) $(CFLAGS) $(INCS) $^ \
		$(NGX_ROOT)/objs/src/core/ngx_palloc.o \
		$(NGX_ROOT)/objs/src/core/ngx_string.o \
		$(NGX_ROOT)/objs/src/os/unix/ngx_alloc.o \
		-o $@ $(LDFLAGS) $(LIBS)

basic_types_pool: basic_types_pool.c
	$(CC) $(CFLAGS) $(INCS) $^ \
		$(NGX_ROOT)/objs/src/core/ngx_palloc.o \
		$(NGX_ROOT)/objs/src/core/ngx_string.o \
		$(NGX_ROOT)/objs/src/os/unix/ngx_alloc.o \
		-o $@ $(LDFLAGS) $(LIBS)

basic_types_array: basic_types_array.c
	$(CC) $(CFLAGS) $(INCS) $^ \
		$(NGX_ROOT)/objs/src/core/ngx_palloc.o \
		$(NGX_ROOT)/objs/src/core/ngx_string.o \
		$(NGX_ROOT)/objs/src/core/ngx_array.o \
		$(NGX_ROOT)/objs/src/os/unix/ngx_alloc.o \
		-o $@ $(LDFLAGS) $(LIBS)

basic_types_queue: basic_types_queue.c
	$(CC) $(CFLAGS) $(INCS) $^ \
		$(NGX_ROOT)/objs/src/core/ngx_palloc.o \
		$(NGX_ROOT)/objs/src/core/ngx_string.o \
		$(NGX_ROOT)/objs/src/core/ngx_queue.o \
		$(NGX_ROOT)/objs/src/os/unix/ngx_alloc.o \
		-o $@ $(LDFLAGS) $(LIBS)

basic_types_hash: basic_types_hash.c
	$(CC) $(CFLAGS) $(INCS) $^ \
		$(NGX_ROOT)/objs/src/core/ngx_palloc.o \
		$(NGX_ROOT)/objs/src/core/ngx_string.o \
		$(NGX_ROOT)/objs/src/core/ngx_hash.o \
		$(NGX_ROOT)/objs/src/core/ngx_array.o \
		$(NGX_ROOT)/objs/src/os/unix/ngx_alloc.o \
		-o $@ $(LDFLAGS) $(LIBS)
