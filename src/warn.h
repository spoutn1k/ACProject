#include "mpi_detection.h"

void warn(basic_block bb, const char* error) {
	printf("[WARNING] %s in basic block %d\n", error, bb->index);
}

void warning(const char* name,const char* warning) {
	printf("[WARNING] in function %s : %s\n",name,warning);
}

void warningline(const char* name,const char* warning,int line) {
	printf("[WARNING] in function %s at line l.%i: %s\n",name,line,warning);
}
