#include "heap-memory.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"

object* _heap_references;

object* heap_references(void) {
	return _heap_references;
}

void reset_heap_references(void) {
	_heap_references = empty_list();
}

void free_heap_references(void) {
	object* ls = heap_references();
	while (!is_empty_list(ls)) {
		free(memory_reference_value(list_first(ls)));
		ls = list_rest(ls);
	}
	reset_heap_references();
}

char* add_heap_memory(object* ls, object* obj, long size, char* reference) {
	_heap_references = init_list_cell(ls, init_memory_reference(obj, size, reference), _heap_references);
	return reference;
}
