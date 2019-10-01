#ifndef MPI_DETECTION_H
#define MPI_DETECTION_H

#include "gccheaders.h"
#include <vector>

enum mpi_collective_code {
#define DEFMPICOLLECTIVES( CODE, NAME ) CODE,
#include "MPI_collectives.def"
	LAST_AND_UNUSED_MPI_COLLECTIVE_CODE
#undef DEFMPICOLLECTIVES
};

/* Name of each MPI collective operations */
#define DEFMPICOLLECTIVES( CODE, NAME ) NAME,
const char *const mpi_collective_name[] = {
#include "MPI_collectives.def"
};
#undef DEFMPICOLLECTIVES

bitmap_head* mpi_calls();

std::vector<int> collectives(basic_block bb);
void aux_reset(function* fun);
int bloc_double_MPI(basic_block bb);

#endif
