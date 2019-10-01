PLUGIN_NAME=mpicheck
CXX=g++
CC=gcc
MPICC=mpicc
LOGFILE=/tmp/log
PLUGIN_FLAGS=-I`$(CC) -print-file-name=plugin`/include -g -Wall -fno-rtti -shared -fPIC
CFLAGS=-g -O3

all: test clean view

test: test2.c
	make -C src
	$(MPICC) test2.c $(CFLAGS) -o /dev/null -fplugin=./src/libplugin_mpicheck.so > $(LOGFILE)
	make -C src clean

view:
	less $(LOGFILE)

clean:
	rm -rf *so
