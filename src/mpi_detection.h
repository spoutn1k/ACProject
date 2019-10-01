#ifndef MPI_DETECTION_H
#define MPI_DETECTION_H

#include <gcc-plugin.h>
#include <tree.h>
#include <plugin-version.h>
#include <basic-block.h>
#include <gimple.h>
#include <tree-pass.h>
#include <context.h>
#include <function.h>
#include <gimple-iterator.h>
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

std::vector<int> collectives(basic_block bb);
void aux_reset(function* fun);
int bloc_double_MPI(basic_block bb);

#endif
