PLUGIN_NAME=mpicheck
CXX=g++
CC=gcc
MPICC=mpicc
PLUGIN_FLAGS=-I`$(CC) -print-file-name=plugin`/include -g -Wall -fno-rtti -shared -fPIC
CFLAGS=-g -O3

all: $(PLUGIN_NAME)

libplugin_%.so: plugin_%.cpp
	$(CXX) $(PLUGIN_FLAGS) -o $@ $<

%: libplugin_%.so test2.c
	$(MPICC) test2.c $(CFLAGS) -o $@ -fplugin=./$< 

clean:
	rm -rf $(EXE) *so
