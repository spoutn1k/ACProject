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

