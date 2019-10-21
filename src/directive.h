#include "gccheaders.h"
#include "c-family/c-pragma.h"
#include "function.h"
#include "tree.h"
#include "tm_p.h"
#include <vector>
#ifndef DIRECTIVE_H
#define DIRECTIVE_H

// Vector storing the registered functions
extern std::vector<const char*> funcname;

// Returns true if the function is registered via mpicoll check
bool is_registered(const char* fname);

// Removes the function from the registered function list
void mark_processed(const char* fname);

// Functions to define and process pragma mpicoll check directives
void register_pragmas(void*, void*);
void handle_pragma_function(cpp_reader *);

// Callback making sure all registered functions were processed
void wrap_mpicoll(void*, void*);

#endif
