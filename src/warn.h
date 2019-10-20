#include "mpi_detection.h"
#include <diagnostic.h>

void divergent_warning(basic_block bb, const int collective) {
	gimple_stmt_iterator gsi;
	gimple *stmt;

	for (gsi = gsi_start_bb(bb); !gsi_end_p (gsi); gsi_next (&gsi))
		stmt = gsi_stmt(gsi);

	warning_at(gimple_location(stmt), 0, "Calls to %s may be avoided from this location", mpi_collective_name[collective]);
}
