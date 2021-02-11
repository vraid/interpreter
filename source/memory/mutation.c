#include "mutation.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"

object* _mutation_references;

object* mutation_references(void) {
	return _mutation_references;
}

void reset_mutation_references(void) {
	_mutation_references = empty_list();
}

void add_mutation_reference(object* ls, object* obj, object* reference) {
	if (obj->location > location_heap) {
		fprintf(stderr, "stack reference at %s\n", location_name[obj->location]);
		exit(0);
	}
	else if ((obj->location == location_heap) && (reference->location == location_stack)) {
		_mutation_references = init_list_cell(ls, obj, _mutation_references);
	}
}
