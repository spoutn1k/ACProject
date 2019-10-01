#include <gcc-plugin.h>
#include <function.h>

static char * cfgviz_generate_filename(function* fun, const char* suffix);
static void cfgviz_internal_dump(function* fun, FILE* out, int td);
void cfgviz_dump(function* fun, const char* suffix, int td);
