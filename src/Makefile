PLUGIN_NAME=mpicheck
CXX=g++
CC=gcc
PLUGIN_FLAGS=-I`$(CC) -print-file-name=plugin`/include -g -Wall -fno-rtti -shared -fPIC

all: $(PLUGIN_NAME)

%.o: %.cpp
	$(CXX) $(PLUGIN_FLAGS) -c $^

%: plugin_%.cpp cfgviz.o
	$(CXX) $(PLUGIN_FLAGS) -o libplugin_$@.so $^

clean:
	rm -rf *so
