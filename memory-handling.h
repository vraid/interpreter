#pragma once

#include "data-structures.h"

#define max_stack_data 1024*1024

void perform_gc(object** root);
char max_stack_references_reached(void);
void add_stack_reference(object* ls, object* obj, object* reference);
#define alloc_stack_reference(a, b) add_stack_reference(alloc_obj(), a, b)

void init_memory_handling();
void init_memory_spaces(int size);
