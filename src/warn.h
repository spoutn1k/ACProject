#include "mpi_detection.h"

void warn(basic_block bb, const char* error) {
	printf("[WARNING] %s in basic block %d\n", error, bb->index);
}
