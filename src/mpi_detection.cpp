#include "mpi_detection.h"

/* Enum to represent the collective operations */
void aux_reset(function* fun) {
	basic_block bb;

	FOR_EACH_BB_FN(bb, fun) {
		bb->aux = 0;
	}
}

int bloc_double_MPI(basic_block bb) {
	gimple_stmt_iterator gsi;
	gimple *stmt;
	int max = 0;

	for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
		/* Get the current statement */
		stmt = gsi_stmt(gsi);

		if (is_gimple_call (stmt)) {
			const char * callee_name ;
			tree t = gimple_call_fndecl(stmt);
			callee_name = IDENTIFIER_POINTER(DECL_NAME(t)) ;
			if (is_present(callee_name) != -1) {
				max++;
				if (max > 1) {
					return 1;
				}
			}
		}
	}

	printf("Found one or less MPI call in bb%d\n", bb->index);
	return 0;
}

