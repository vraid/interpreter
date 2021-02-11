#pragma once

#include "data-structures.h"

object* mutation_references(void);
void reset_mutation_references(void);
void add_mutation_reference(object* ls, object* obj, object* reference);
#define alloc_mutation_reference(a, b) add_mutation_reference(alloc_obj(), a, b)
