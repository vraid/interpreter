#pragma once

#include "data-structures.h"

object* heap_references(void);
void reset_heap_references(void);
void free_heap_references(void);
char* add_heap_memory(object* ls, object* obj, long size, char* reference);
#define alloc_heap_memory(a) add_heap_memory(alloc_obj(), alloc_obj(), a, malloc(a))
