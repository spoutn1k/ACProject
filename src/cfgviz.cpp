#include "cfgviz.h"

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

