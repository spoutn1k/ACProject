PLUGIN_NAME=mpicheck
CXX=g++_910
CC=gcc_910
PLUGIN_FLAGS=-I`$(CC) -print-file-name=plugin`/include -Wall -fno-rtti -shared -fPIC
OBJECTS=cfgviz.o mpi_detection.o domination.o directive.o path.o

all: $(PLUGIN_NAME)

%.o: %.cpp
	$(CXX) $(PLUGIN_FLAGS) -c $^

%: plugin_%.cpp $(OBJECTS)
	$(CXX) $(PLUGIN_FLAGS) -o libplugin_$@.so $^

clean:
	rm -rf *so
