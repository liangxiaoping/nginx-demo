#!/bin/sh

set -x

CFLAGS="-O2 -g -Wall -Wno-deprecated"
export CFLAGS
gcc -I ../../../objs/ -I ../../os/unix/ $CFLAGS basic_types_int.c -o basic_types_int
gcc -c -O -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -Werror -g -I ../../../objs/ -I ../../os/unix/ basic_types_str.c -I../../core/ -I../../event/ -I../../os/ -o basic_types_str.o
gcc -o basic_types_str basic_types_str.o ../../../objs/src/core/ngx_{string,palloc}.o ../../../objs/src/os/unix/ngx_alloc.o -lcrypt -lpcre -lcrypto -lz
