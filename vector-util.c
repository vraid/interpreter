#include "vector-util.h"

#include <stdlib.h>
#include "data-structures.h"
#include "sequences.h"
#include "global-variables.h"
#include "object-init.h"
#include "list-util.h"
#include "call.h"
#include "delist.h"

object* vector_ref(object* vec, int n) {
	if (is_vector_iterator(vec)) {
		return vector_ref(vector_iterator_vector(vec), n + vector_iterator_index(vec));
	}
	else {
		object** data = vector_data(vec);
		int length = vector_length(vec);
		
		if (n < 0 || n >= length) {
			fprintf(stderr, "invalid vector index: %i of %i\n", n, length);
			return no_object();
		}
		else {
			 return data[n];
		}
	}
}

object* vector_to_list(object* args, object* cont) {
	object* vec;
	delist_1(args, &vec);
	
	int length = vector_length(vec);
	object* list = alloca(sizeof(object) * length);
	
	object* first = empty_list();
	
	int i;
	for (i = length-1; i+1 > 0; i--) {
		init_list_cell(&list[i], vector_ref(vec, i), first);
		first = &list[i];
	}
	return call_cont(cont, first);
}

object* list_to_vector(object* args, object* cont) {
	object* list;
	delist_1(args, &list);
	
	int length = list_length(list);
	
	object** data = alloca(sizeof(object*) * length);
	
	int count = 0;
	object* ls = list;
	
	while (!is_empty_list(ls)) {
		data[count] = list_first(ls);
		ls = list_rest(ls);
		count++;
	}
	
	object vector;
	init_vector(&vector, length, data);
	
	object iterator;
	object* iter = first_iterator(&iterator, &vector);
	
	return call_cont(cont, iter);
}

void init_vector_util_procedures(void) {
	init_primitive_procedure(&vector_to_list_proc, &vector_to_list);
	init_primitive_procedure(&list_to_vector_proc, list_to_vector);
}
