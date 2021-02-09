#pragma once

#include "data-structures.h"

#define max_stack_data 1024*1024

void perform_gc(object** root);
void add_mutation_reference(object* ls, object* obj, object* reference);
#define alloc_mutation_reference(a, b) add_mutation_reference(alloc_obj(), a, b)
void add_malloc_reference(object* ls, object* obj, long size, char* reference);
#define alloc_malloc_reference(a, b) add_malloc_reference(alloc_obj(), alloc_obj(), a, b)
void add_repl_scope_reference(object* ls, object* obj);
#define alloc_repl_scope_reference(a) add_repl_scope_reference(alloc_obj(), a)

object* rewound_repl_scope_reference(object* key);

void init_memory_handling();
void init_memory_spaces(int size);
