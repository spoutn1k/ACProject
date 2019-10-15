#include "gccheaders.h"
#include "c-family/c-pragma.h"
#include "function.h"
#include "tree.h"
#include "tm_p.h"
#include <vector>
#ifndef DIRECTIVE_H
#define DIRECTIVE_H

void register_pragmas(void*, void*);

void handle_pragma_function(cpp_reader *);
#endif
