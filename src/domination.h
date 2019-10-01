#include <gcc-plugin.h>
#include <plugin-version.h>
#include <basic-block.h>
#include <dominance.h>
#include <function.h>

void init_dom();

void print_dominated(basic_block bb) ;

void free_dom() ;

void init_postdom() ;

void print_postdominated(basic_block bb) ;

void free_postdom() ;

