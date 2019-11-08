CXX=g++
CC=gcc
MPICC=mpicc
LOGFILE=/tmp/log
PLUGIN_FLAGS=-I`$(CC) -print-file-name=plugin`/include -g -Wall -fno-rtti -shared -fPIC
CFLAGS=-g -O3

all: test

test: intervals.c
	make -C src
	$(MPICC) $^ $(CFLAGS) -o /dev/null -fplugin=./src/libplugin_mpicheck.so

clean:
	rm -rf *so
