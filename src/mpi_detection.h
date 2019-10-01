#include <gcc-plugin.h>
#include <plugin-version.h>
#include <tree.h>
#include <basic-block.h>
#include <gimple.h>
#include <tree-pass.h>
#include <context.h>
#include <function.h>
#include <gimple-iterator.h>

enum mpi_collective_code {
#define DEFMPICOLLECTIVES( CODE, NAME ) CODE,
#include "MPI_collectives.def"
	LAST_AND_UNUSED_MPI_COLLECTIVE_CODE
#undef DEFMPICOLLECTIVES
} ;

/* Name of each MPI collective operations */
#define DEFMPICOLLECTIVES( CODE, NAME ) NAME,
const char *const mpi_collective_name[] = {
#include "MPI_collectives.def"
} ;
#undef DEFMPICOLLECTIVES

#define DEFMPICOLLECTIVES( CODE, NAME ) if(!strcmp(test, NAME)){return index;}else{index++;};
int is_present(const char* test) {
	int index = 0;
#include "MPI_collectives.def"
	return -1;
};

void aux_reset(function* fun);
int bloc_double_MPI(basic_block bb);
