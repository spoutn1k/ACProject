#include "cfgviz.h"
#include <cstring>

/* Build a filename (as a string) based on function name */
static char * cfgviz_generate_filename( function * fun) {
	char * target_filename ;

	target_filename = (char *)xmalloc( 1024 * sizeof( char ) ) ;

	snprintf( target_filename, 1024, "%s_%s_%d.dot",
			current_function_name(),
			LOCATION_FILE( fun->function_start_locus ),
			LOCATION_LINE( fun->function_start_locus));

	return target_filename ;
}

void cfgviz_generate_label(basic_block bb, char* buffer) {
	gimple_stmt_iterator gsi;
	gimple *stmt;
	sprintf(buffer, "BB%d\n", bb->index);

	for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
		/* Get the current statement */
		stmt = gsi_stmt(gsi);

		if (is_gimple_call (stmt)) {
			tree t = gimple_call_fndecl(stmt);
			strcat(buffer, IDENTIFIER_POINTER(DECL_NAME(t)));
			strcat(buffer, "\n");
		}
	}
}

/* Dump the graphviz representation of function 'fun' in file 'out' */
static void cfgviz_internal_dump( function * fun, FILE * out) {
	basic_block bb;
	char buffer[1000];
	fun = (fun + 0);

	// Print the header line and open the main graph
	fprintf(out, "Digraph G{\n");

	FOR_ALL_BB_FN(bb,cfun) {
		memset(buffer, 0, 1000);
		cfgviz_generate_label(bb, buffer);

		fprintf( out,
				"%d [label=\"%s",
				bb->index,
				buffer
			   );

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

void cfgviz_dump( function * fun) {
	char * target_filename ;
	FILE * out ;

	target_filename = cfgviz_generate_filename( fun);

	printf( "[GRAPHVIZ] Generating CFG of function %s in file <%s>\n",
			current_function_name(), target_filename ) ;

	out = fopen( target_filename, "w" ) ;

	cfgviz_internal_dump(fun, out) ;

	fclose( out ) ;
	free( target_filename ) ;
}

