#include "domination.h"

void init_dom() {
	calculate_dominance_info(CDI_DOMINATORS);	
}

void print_dominated(basic_block bb) {
	vec<basic_block> vecdom = get_all_dominated_blocks(CDI_DOMINATORS,bb);
	printf("BB n%d : ",bb->index);
	for (unsigned int ix=1;ix<vecdom.length();ix++) 
		printf("%d,",vecdom[ix]->index);
	printf("\n");
}

void free_dom() {
	free_dominance_info(CDI_DOMINATORS);
}

void post_dom_frontier(bitmap_head* frontiers) {
	edge arc;
	edge_iterator edge_iterator;
	basic_block bb, domsb;

	FOR_EACH_BB_FN (bb, cfun) {
		if (EDGE_COUNT (bb->succs) >= 2) {
			domsb = get_immediate_dominator (CDI_POST_DOMINATORS, bb);
			FOR_EACH_EDGE (arc, edge_iterator, bb->succs) {
				basic_block runner = arc->dest;
				if (runner == EXIT_BLOCK_PTR_FOR_FN (cfun))
					continue;

				while (runner != domsb) {
					if (!bitmap_set_bit (&frontiers[runner->index], bb->index)) {
						break;
					}
					runner = get_immediate_dominator (CDI_POST_DOMINATORS, runner);
				}
			}
		}
	}
}

void init_postdom() {
	calculate_dominance_info(CDI_POST_DOMINATORS);	
}

void print_postdominated(basic_block bb) {
	vec<basic_block> vecdom = get_all_dominated_blocks(CDI_POST_DOMINATORS,bb);
	printf("%d\n",vecdom.length());
	for (unsigned int ix=1;ix<vecdom.length();ix++)  
		printf("%d,",vecdom[ix]->index);
	printf("\n");
}

void free_postdom() {
	free_dominance_info(CDI_POST_DOMINATORS);
}

bitmap_head* init_frontiers() {
	basic_block bb;
	bitmap_head* frontiers = XNEWVEC (bitmap_head, last_basic_block_for_fn (cfun));

	FOR_EACH_BB_FN(bb, cfun)
		bitmap_initialize(&frontiers[bb->index], &bitmap_default_obstack);

	return frontiers;
}

void release_frontiers(bitmap_head* frontiers) {
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun)
		bitmap_release(&frontiers[bb->index]);
}

void pdf_set(bitmap_head* res, const bitmap_head* frontiers, bitmap_head* set) {
	basic_block bb;
	unsigned int bb_index;
	bitmap_iterator biter;
	bitmap_head tmp_bm;
	bitmap_initialize(&tmp_bm, &bitmap_default_obstack);
	
	// Join all the PDFs of the set's nodes
	EXECUTE_IF_SET_IN_BITMAP(set, 0, bb_index, biter)
		bitmap_ior_into(res, &frontiers[bb_index]);

	// Add the PDFs of all the nodes of the jointure to the jointure
	do {
		bitmap_copy(&tmp_bm, res);
		EXECUTE_IF_SET_IN_BITMAP(&tmp_bm, 0, bb_index, biter)
			bitmap_ior_into(res, &frontiers[bb_index]);
	} while (!bitmap_equal_p(res, &tmp_bm));

	// We join the PDFs of every node NOT in the set, into tmp_bm
	bitmap_clear(&tmp_bm);
	FOR_EACH_BB_FN(bb, cfun)
		if (!bitmap_bit_p(set, bb->index))
			bitmap_ior_into(&tmp_bm, &frontiers[bb->index]);

	// Select all the nodes that are in the PDFs of nodes NOT in the set
	bitmap_and_into(res, &tmp_bm);

	bitmap_release(&tmp_bm);
}
