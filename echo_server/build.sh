#!/bin/sh

set -x

CFLAGS="-O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -Wunused-variable -g"
export CFLAGS
gcc -I .. $CFLAGS -o echo_server echo_server.c

