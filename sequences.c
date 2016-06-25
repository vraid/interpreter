#include "sequences.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "vector-util.h"

int sequence_length(object* obj) {
	switch(obj->type) {
		case type_list: return list_length(obj);
		case type_vector_iterator: return vector_iterator_length(obj);
		default:
			fprintf(stderr, "invalid sequence type: %s\n", type_name[obj->type]);
			exit(0);
	}
}

object* sequence_first(object* obj) {
	switch (obj->type) {
		case type_list: return list_first(obj);
		case type_vector_iterator: return vector_ref(obj, 0);
		default:
			fprintf(stderr, "invalid sequence type: %s\n", type_name[obj->type]);
			exit(0);
	}
}

object* first_vector_iterator(object* iter, object* obj) {
	if (vector_length(obj) == 0) {
		return empty_list();
	}
	else {
		init_vector_iterator(iter, 0, obj);
		return iter;
	}
}

object* first_iterator(object* iter, object* obj) {
	switch (obj->type) {
		case type_list: return obj;
		case type_vector: return first_vector_iterator(iter, obj);
		default:
			fprintf(stderr, "invalid sequence type: %s\n", type_name[obj->type]);
			return no_object();
	}
}

object* next_vector_iterator(object* next, object* current) {
	int n = 1 + vector_iterator_index(current);
	object* vector = vector_iterator_vector(current);
	if (n == vector_length(vector)) {
		next = empty_list();
	}
	else {
		init_vector_iterator(next, n, vector);
	}
	return next;
}

object* next_iterator(object* next, object* current) {
	switch (current->type) {
		case type_list: return list_rest(current);
		case type_vector_iterator: return next_vector_iterator(next, current);
		default:
			fprintf(stderr, "invalid iterator type: %s\n", type_name[current->type]);
			return no_object();
	}
}
