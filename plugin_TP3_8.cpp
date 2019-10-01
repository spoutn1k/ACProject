#include <gcc-plugin.h>
#include <plugin-version.h>
#include <tree.h>
#include <basic-block.h>
#include <gimple.h>
#include <tree-pass.h>
#include <context.h>
#include <function.h>
#include <gimple-iterator.h>
#define GDUMP

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

/* Enum to represent the collective operations */
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

/* Build a filename (as a string) based on function name */
static char * cfgviz_generate_filename( function * fun, const char * suffix ) {
	char * target_filename ;

	target_filename = (char *)xmalloc( 1024 * sizeof( char ) ) ;

	snprintf( target_filename, 1024, "%s_%s_%d_%s.dot",
			current_function_name(),
			LOCATION_FILE( fun->function_start_locus ),
			LOCATION_LINE( fun->function_start_locus ),
			suffix ) ;

	return target_filename ;
}

/* Dump the graphviz representation of function 'fun' in file 'out' */
static void cfgviz_internal_dump( function * fun, FILE * out, int td ) {
	basic_block bb;
	fun = (fun + 0);
	td = (td + 0);

	// Print the header line and open the main graph
	fprintf(out, "Digraph G{\n");

	FOR_ALL_BB_FN(bb,cfun) {
		fprintf( out,
				"%d [label=\"BB %d",
				bb->index,
				bb->index
			   );

		if ((long)bb->aux > 0)
			fprintf(out, "\n%s", mpi_collective_name[((long)bb->aux)-1]);

		fprintf(out, "\" shape=ellipse]\n");
		// Process output edges 
		edge_iterator eit;
		edge e;

		FOR_EACH_EDGE( e, eit, bb->succs ) {
			const char *label = "";
			if( e->flags == EDGE_TRUE_VALUE )
				label = "true";
			else if( e->flags == EDGE_FALSE_VALUE )
				label = "false";

			fprintf( out, "%d -> %d [color=red label=\"%s\"]\n",
					bb->index, e->dest->index, label ) ;
		}
	}
	// Close the main graph
	fprintf(out, "}\n");
}

void cfgviz_dump( function * fun, const char * suffix, int td ) {
	char * target_filename ;
	FILE * out ;

	target_filename = cfgviz_generate_filename( fun, suffix ) ;

	printf( "[GRAPHVIZ] Generating CFG of function %s in file <%s>\n",
			current_function_name(), target_filename ) ;

	out = fopen( target_filename, "w" ) ;

	cfgviz_internal_dump( fun, out, td ) ;

	fclose( out ) ;
	free( target_filename ) ;
}

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

/* My new pass inheriting from regular gimple pass */
class my_pass : public gimple_opt_pass {
	public:
		/* constructor */
		my_pass (gcc::context *ctxt): gimple_opt_pass(my_pass_data, ctxt) {}

		my_pass *clone () {
			return new my_pass(g);
		}

		bool gate (function *fun) {
			printf("[gate] processing function: %s\n", function_name(fun));
			return true;
		}

		unsigned int execute (function *fun) {
			printf("[execute] parsing function: %s\n", function_name(fun));
			basic_block bb;
			gimple_stmt_iterator gsi;
			gimple *stmt;

			FOR_EACH_BB_FN(bb, fun) {
				gsi = gsi_start_bb (bb);
				stmt = gsi_stmt (gsi);
				bb->aux = (void*) (long) -1;
				//printf("\t|-> parsing bb%d: l%d - %p\n", bb->index, gimple_lineno(stmt), bb->aux);
				gimple_stmt_iterator gsi;

				/* Iterate on gimple statements in the current basic block */
				for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
					/* Get the current statement */
					stmt = gsi_stmt (gsi);

					if (is_gimple_call (stmt)) {
						const char * callee_name ;
						tree t = gimple_call_fndecl( stmt ) ;
						callee_name = IDENTIFIER_POINTER(DECL_NAME(t)) ;
						int index = is_present(callee_name);
						bb->aux = (void*) (long) (index + 1);
						if (index != -1 && bloc_double_MPI(bb)) {
							printf("\t|\t|-> function call: \"%s\" (%d)\n", callee_name, index);
							split_block(bb, stmt);
						}
					}
				}
			}

#ifdef GDUMP
			/* Skip functions if its a system header*/
			if ( !in_system_header_at( fun->function_start_locus ) )
				cfgviz_dump( fun, "0_ini", /*TD*/3 ) ;
#endif

			aux_reset(fun);
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

	printf( "plugin_init: Pass added...\n" ) ;

	return 0;
}
