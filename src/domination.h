#ifndef DOMINATION_H
#define DOMINATION_H

#include "gccheaders.h"

void init_dom();

void print_dominated(basic_block bb) ;

void free_dom() ;

void init_postdom() ;

void print_postdominated(basic_block bb) ;

void free_postdom() ;

#endif
