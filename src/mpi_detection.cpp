#include "mpi_detection.h"

#define DEFMPICOLLECTIVES( CODE, NAME ) if(!strcmp(func_name, NAME)){return index;}else{index++;};
int is_mpi(const char* func_name) {
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

int bloc_double_mpi(basic_block bb) {
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

int collective(basic_block bb) {
	std::vector<int> detected = collectives(bb);

	if (detected.size() != 1)
		return -1;

	return detected[0];
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

void print_mpi_calls() {
	bitmap_head* calls = mpi_calls();

	printf("Used calls\n");
	for (int i = 0; i < LAST_AND_UNUSED_MPI_COLLECTIVE_CODE; i++) {
		printf("%s: ", mpi_collective_name[i]);
		bitmap_print(stdout, &calls[i], "", "\n");
	}

	release_calls(calls);
}

void isolate_mpi() {
	basic_block bb;
	gimple_stmt_iterator gsi;
	gimple *stmt;
	int to_cut;

	FOR_EACH_BB_FN(bb, cfun) {
		to_cut = bloc_double_mpi(bb);

		for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
			stmt = gsi_stmt(gsi);

			if (is_gimple_call(stmt)) {
				tree t = gimple_call_fndecl(stmt);
				const char * callee_name = IDENTIFIER_POINTER(DECL_NAME(t)) ;

				if (to_cut && is_mpi(callee_name) != -1)
					split_block(bb, stmt);
			}
		}
	}
}
