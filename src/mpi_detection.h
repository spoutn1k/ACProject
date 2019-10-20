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

// Detect all the mpi calls made in the program
// Returns a bitmap vector, which size equals the number of defined collectives
// The bits set in the bitmap k indicate that the kth collective is used in
// the corresponding basic_block
bitmap_head* mpi_calls();
void release_calls(bitmap_head* mpi_calls);

// A vector of collective codes used in `bb`
// We don't use a bitmap because a collective may be called more than once
std::vector<int> collectives(basic_block bb);

// The first collective called in `bb`
int collective(basic_block bb);

// True if there is more than one collective in the basic_block
bool bloc_double_mpi(basic_block bb);

// This method will slice basic blocks tu ensure that each basic_block contains
// AT MOST one collective
void isolate_mpi();

// This just prints the results of mpi_calls
void print_mpi_calls();

#endif
