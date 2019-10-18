#ifndef DOMINATION_H
#define DOMINATION_H

#include "gccheaders.h"
#include <vector>
#include "mpi_detection.h"
void init_dom();
void print_dominated(basic_block bb) ;
void free_dom() ;

void post_dom_frontier(bitmap);

void init_postdom();
void print_postdominated(basic_block bb);
void free_postdom();

bitmap_head* init_frontiers();
void release_frontiers(bitmap_head* frontiers);
void pdf_set(bitmap_head*, const bitmap_head*, bitmap_head*);
bitmap_head* compute_pdf_sets(bitmap_head* sets);
#endif
