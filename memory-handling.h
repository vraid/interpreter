#pragma once

#include "data-structures.h"

#define max_stack_data 1024*1024

void perform_gc(object** root);
void add_mutation_reference(object* ls, object* obj, object* reference);
#define alloc_mutation_reference(a, b) add_mutation_reference(alloc_obj(), a, b)

void init_memory_handling();
void init_memory_spaces(int size);
