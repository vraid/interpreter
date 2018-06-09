#pragma once

#include "data-structures.h"

#define max_stack_data 1024*1024

void perform_gc(object** root);
char max_stack_references_reached(void);
void add_stack_reference(object* obj, object* reference);

void init_memory_spaces();
