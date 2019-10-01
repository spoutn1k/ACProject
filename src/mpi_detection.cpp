#include "mpi_detection.h"

#define DEFMPICOLLECTIVES( CODE, NAME ) if(!strcmp(test, NAME)){return index;}else{index++;};
int is_present(const char* test) {
	int index = 0;
#include "MPI_collectives.def"
	return -1;
};
#undef DEFMPICOLLECTIVES

/* Enum to represent the collective operations */
void aux_reset(function* fun) {
	basic_block bb;

	FOR_EACH_BB_FN(bb, fun) {
		bb->aux = 0;
	}
}

int bloc_double_MPI(basic_block bb) {
	return collectives(bb).size() > 1;
}

std::vector<int> collectives(basic_block bb) {
	gimple_stmt_iterator gsi;
	gimple *stmt;
	std::vector<int> detected_codes = {};

	for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
		/* Get the current statement */
		stmt = gsi_stmt(gsi);

		if (is_gimple_call (stmt)) {
			tree t = gimple_call_fndecl(stmt);
			const char * callee_name = IDENTIFIER_POINTER(DECL_NAME(t)) ;

			for (int i = 0; i < LAST_AND_UNUSED_MPI_COLLECTIVE_CODE; i++)
				if (!strcmp(callee_name, mpi_collective_name[i]))
					detected_codes.push_back(i);
		}
	}

	return detected_codes;
}

bitmap_head* mpi_calls() {
	basic_block bb;
	bitmap_head* mpi_calls = XNEWVEC(bitmap_head, LAST_AND_UNUSED_MPI_COLLECTIVE_CODE);

	for (int i = 0; i < LAST_AND_UNUSED_MPI_COLLECTIVE_CODE; i++)
		bitmap_initialize(&mpi_calls[i], &bitmap_default_obstack);

	FOR_EACH_BB_FN(bb, cfun) {
		for (int n : collectives(bb))
			bitmap_set_bit(&mpi_calls[n], bb->index);
	}

	return mpi_calls;
}

void release_calls(bitmap_head* mpi_calls) {
	for (int i = 0; i < LAST_AND_UNUSED_MPI_COLLECTIVE_CODE; i++)
		bitmap_release(&mpi_calls[i]);
}

