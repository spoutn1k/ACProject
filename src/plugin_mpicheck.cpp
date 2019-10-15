#include <iostream>
#include "mpi_detection.h"
#include "cfgviz.h"
#include "domination.h"
#include "warn.h"
#include "directive.h"
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

		bool gate (function *fun) {
			for (int i = 0 ; i < funcname.size(); i++) {
				if (!strcmp(funcname[i],function_name(fun))) {
					funcname.erase(funcname.begin()+i);
					printf("[gate] processing function: %s\n", function_name(fun));
					return true;	
				}
			}
			return false;
		}

		unsigned int execute (function *fun) {
			printf("[execute] parsing function: %s\n", function_name(fun));
			basic_block bb;

			init_postdom();

			bitmap_head* frontiers = init_frontiers();
			post_dom_frontier(frontiers);
			FOR_EACH_BB_FN(bb, fun) {
				warn(bb, "Parsing");
			}
			bitmap_head set, res;
			bitmap_initialize(&set, &bitmap_default_obstack);
			bitmap_initialize(&res, &bitmap_default_obstack);
			bitmap_set_bit(&set, 4);
			bitmap_set_bit(&set, 5);
			pdf_set(&res, frontiers, &set);

			bitmap_print(stdout, &res, "Res: ", "\n");

			free_postdom();

			cfgviz_dump(fun);

			return 0;
		}
};

/* Main entry point for plugin */
int plugin_init(struct plugin_name_args * plugin_info,
		struct plugin_gcc_version * version) {
	struct register_pass_info my_pass_info;

	printf( "plugin_init: Entering...\n" ) ;

	/* First check that the current version of GCC is the right one */
	if (!plugin_default_version_check(version, &gcc_version))
		return 1;

	printf( "plugin_init: Check ok...\n" ) ;

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

	register_callback(plugin_info->base_name, PLUGIN_PRAGMAS, register_pragmas, NULL);

	printf( "plugin_init: Pass added...\n" ) ;

	return 0;
}
