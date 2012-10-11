#!/bin/bash

#
# Use this makefile, we have to compile the nginx before starting compile this demo.
# We just use nginx to write demo programs.
#

NGX_ROOT= ../../nginx-1.2.4
CC= gcc

CFLAGS=-g3 -O0 -W -Wall -Werror -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value 

INCLUDEPATH= -I. \
		     -I$(NGX_ROOT)/src/core \
		     -I$(NGX_ROOT)/src/event \
		     -I$(NGX_ROOT)/src/event/modules \
		     -I$(NGX_ROOT)/src/os/unix \
		     -I$(NGX_ROOT)/objs \
		     -I$(NGX_ROOT)/src/http \
		     -I$(NGX_ROOT)/src/http/modules
        
OBJPATH= $(NGX_ROOT)/objs/*.o \
         $(NGX_ROOT)/objs/src/os/unix/*.o \
         $(NGX_ROOT)/objs/src/core/*.o \
         $(NGX_ROOT)/objs/src/event/*.o \
         $(NGX_ROOT)/objs/src/event/modules/*.o \
         $(NGX_ROOT)/objs/src/http/*.o \
         $(NGX_ROOT)/objs/src/http/modules/*.o 


LDFLAGS= 
LIBS= -lstdc++ -lpthread  -lcrypt -lpcre -lcrypto -lz -lssl

SOURCE=$(wildcard *.c)
DIRECTORIES=$(notdir $(SOURCE))
OBJECTS=$(patsubst %.c,%.o,$(DIRECTORIES) )
TARGETS=$(patsubst %.c,%,$(DIRECTORIES) )

all: $(TARGETS)

$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $(INCLUDEPATH) $< -o $@

$(TARGETS):$(OBJECTS)
	$(CC) $(OBJPATH) $(LDFLAGS) $(LIBS) -o $@ 

.PHONEY:clean

clean:
	rm -f $(TARGETS) $(OBJECTS)