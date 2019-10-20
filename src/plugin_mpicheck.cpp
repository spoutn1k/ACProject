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
	"NEW_PASS", /* name */
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
			for (long unsigned int i = 0 ; i < funcname.size(); i++) {
				if (!strcmp(funcname[i],function_name(fun))) {
					funcname.erase(funcname.begin()+i);
					printf("[gate] processing function: %s\n", function_name(fun));
					return true;	
				}
			}
			return false;
		}

		unsigned int execute (function* fun) {
			printf("[execute] parsing function: %s\n", function_name(fun));

			isolate_mpi();
			bitmap_head* sets = mpi_calls();
			bitmap_head* res = compute_pdf_sets(sets);	

			cfgviz_dump(fun);
			bitmap_iterator bi;
			unsigned int bb_index;
			basic_block bb;

			for (int i = 0; i < LAST_AND_UNUSED_MPI_COLLECTIVE_CODE; i++) {
				EXECUTE_IF_SET_IN_BITMAP(&res[i], 0, bb_index, bi) {
					bb = BASIC_BLOCK_FOR_FN(fun, bb_index);
					PathFinder checker(bb);
					if (!checker.common_path() && bb->loop_father->num == 0)
						divergent_warning(bb, i);
				}
			}

			release_calls(res);
			release_calls(sets);

			return 0;
		}
};
//print_graph_cfg("/tmp/graph", fun);

/* Main entry point for plugin */
int plugin_init(struct plugin_name_args * plugin_info,
		struct plugin_gcc_version * version) {
	struct register_pass_info my_pass_info;

	//printf( "plugin_init: Entering...\n" ) ;

	/* First check that the current version of GCC is the right one */
	if (!plugin_default_version_check(version, &gcc_version))
		return 1;

	//printf( "plugin_init: Check ok...\n" ) ;

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
			PLUGIN_PASS_MANAGER_SETUP,
			NULL,
			&my_pass_info);

	register_callback(plugin_info->base_name,
			PLUGIN_PRAGMAS,
			register_pragmas,
			NULL);

	//printf( "plugin_init: Pass added...\n" ) ;
	return 0;
}
