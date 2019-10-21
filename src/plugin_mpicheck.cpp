#include <iostream>
#include "mpi_detection.h"
#include "cfgviz.h"
#include "domination.h"
#include "warn.h"
#include "directive.h"
#include "path.h"
#include <graph.h>
#include <plugin-version.h>

/* Global variable required for plugin to execute */
int plugin_is_GPL_compatible;

/* Global object (const) to represent my pass */
const pass_data my_pass_data =
{
	GIMPLE_PASS, /* type */
	"MPI_COLL", /* name */
	OPTGROUP_NONE, /* optinfo_flags */
	TV_OPTIMIZE, /* tv_id */
	0, /* properties_required */
	0, /* properties_provided */
	0, /* properties_destroyed */
	0, /* todo_flags_start */
	0, /* todo_flags_finish */
};

/* My new pass inheriting from regular gimple pass */
class my_pass : public gimple_opt_pass {
	public:
		/* constructor */
		my_pass (gcc::context *ctxt): gimple_opt_pass(my_pass_data, ctxt) {}

		my_pass *clone () {
			return new my_pass(g);
		}

		bool gate (function* fun) {
			if (is_registered(function_name(fun))) {
				mark_processed(function_name(fun));
				return true;
			}

			return false;
		}

		unsigned int execute (function* fun) {
			basic_block bb;
			bitmap_iterator bi;
			unsigned int bb_index;

			// Isolate the MPI calls on a basic block each
			isolate_mpi();
			bitmap_head* sets = mpi_calls();
			bitmap_head* res = compute_pdf_sets(sets);	

			//cfgviz_dump(fun);

			// For every defined collective
			for (int i = 0; i < LAST_AND_UNUSED_MPI_COLLECTIVE_CODE; i++) {
				// For every bb in its post dominance set
				EXECUTE_IF_SET_IN_BITMAP(&res[i], 0, bb_index, bi) {
					bb = BASIC_BLOCK_FOR_FN(fun, bb_index);
					PathFinder checker(bb);
					// If the mpi sequence is not unique and its not a loop
					if (!checker.common_path() && bb->loop_father->num == 0)
						divergent_warning(bb, i); // Raise a warning
				}
			}

			release_calls(res);
			release_calls(sets);

			return 0;
		}
};

/* Main entry point for plugin */
int plugin_init(struct plugin_name_args * plugin_info,
		struct plugin_gcc_version * version) {
	struct register_pass_info my_pass_info;

	/* First check that the current version of GCC is the right one */
	if (!plugin_default_version_check(version, &gcc_version))
		return 1;

	/* Declare and build my new pass */
	my_pass p(g);

	/* Fill info on my pass 
	   (insertion after the pass building the CFG) */
	my_pass_info.pass = &p;
	my_pass_info.reference_pass_name = "cfg";
	my_pass_info.ref_pass_instance_number = 0;
	my_pass_info.pos_op = PASS_POS_INSERT_AFTER;

	/* Add my pass to the pass manager */
	register_callback(plugin_info->base_name,
			PLUGIN_PRAGMAS,
			register_pragmas,
			NULL);

	register_callback(plugin_info->base_name,
			PLUGIN_PASS_MANAGER_SETUP,
			NULL,
			&my_pass_info);

	register_callback(plugin_info->base_name,
			PLUGIN_ALL_PASSES_END,
			wrap_mpicoll,
			NULL);

	return 0;
}
