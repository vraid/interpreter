#include "vectors.h"

#include <stdlib.h>
#include "data-structures.h"
#include "sequences.h"
#include "global-variables.h"
#include "object-init.h"
#include "list-util.h"
#include "call.h"
#include "delist.h"
#include "lists.h"
#include "sequences.h"
#include "higher-order.h"
#include "syntax-base.h"
#include "print.h"

object* vector(object* args, object* cont) {
	object* vector_call = alloc_call(&list_to_vector_proc, empty_list(), cont);	
	object* list_call = alloc_call(syntax_procedure_obj(syntax_list), args, alloc_cont(vector_call));
	
	return perform_call(list_call);
}

object* vector_internal_ref(object* vec, int n) {
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

object* vector_ref(object* vec, int n) {
	return vector_internal_ref(vector_iterator_vector(vec), n + vector_iterator_index(vec));
}

object* vector_first(object* vec) {
	
	return vector_ref(vec, 0);
}

int vector_find(object* vec, object* a) {
	for (int n = 0; n < vector_iterator_length(vec); n++) {
		if (vector_ref(vec, n) == a) {
			return n;
		}
	}
	return -1;
}

object vector_rest_proc;

object* vector_rest(object* args, object* cont) {
	object* vec;
	delist_1(args, &vec);
	
	int n = 1 + vector_iterator_index(vec);
	object* vector = vector_iterator_vector(vec);
	if (n == vector_length(vector)) {
		return call_cont(cont, end_vector_iterator());
	}
	else {
		object* next = alloc_vector_iterator(n, vector);
		return call_cont(cont, next);
	}
}

object* vector_to_list(object* args, object* cont) {
	object* vec;
	delist_1(args, &vec);
	
	int length = vector_iterator_length(vec);
	object* list = alloc_objects(length);
	
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
	
	if (length == 0) {
		return call_cont(cont, end_vector_iterator());
	}
	else {
		object** data = alloc_bytes(sizeof(object*) * length);
		
		int count = 0;
		object* ls = list;
		
		while (!is_empty_list(ls)) {
			data[count] = list_first(ls);
			ls = list_rest(ls);
			count++;
		}
		
		object* vector = alloc_vector(length, data);
		object* iter = alloc_vector_iterator(0, vector);
		
		return call_cont(cont, iter);
	}
}

object vector_list_map_proc;

object* vector_list_map(object* args, object* cont) {
	object* ls;
	object* function;
	object* trace;
	delist_3(args, &ls, &function, &trace);
	
	object* next_args = alloc_list_3(function, ls, trace);
	object* next_call = alloc_call(&list_map_proc, next_args, cont);
	
	return perform_call(next_call);
}

object vector_map_proc;

object* vector_map(object* args, object* cont) {
	object* function;
	object* vec;
	object* trace;
	delist_3(args, &function, &vec, &trace);
	
	object* vector_call = alloc_call(&list_to_vector_proc, empty_list(), cont);
	
	object* next_args = alloc_list_2(function, trace);
	object* next_call = alloc_call(&vector_list_map_proc, next_args, alloc_cont(vector_call));
	
	object* list_args = alloc_list_1(vec);
	object* list_call = alloc_call(&vector_to_list_proc, list_args, alloc_cont(next_call));
	
	return perform_call(list_call);
}

object vector_list_filter_proc;

object* vector_list_filter(object* args, object* cont) {
	object* ls;
	object* function;
	object* trace;
	delist_3(args, &ls, &function, &trace);
	
	object* next_args = alloc_list_3(function, ls, trace);
	object* next_call = alloc_call(&list_filter_proc, next_args, cont);
	
	return perform_call(next_call);
}

object vector_filter_proc;

object* vector_filter(object* args, object* cont) {
	object* function;
	object* vec;
	object* trace;
	delist_3(args, &function, &vec, &trace);
	
	object* vector_call = alloc_call(&list_to_vector_proc, empty_list(), cont);
	
	object* next_args = alloc_list_2(function, trace);
	object* next_call = alloc_call(&vector_list_filter_proc, next_args, alloc_cont(vector_call));
	
	object* list_args = alloc_list_1(vec);
	object* list_call = alloc_call(&vector_to_list_proc, list_args, alloc_cont(next_call));
	
	return perform_call(list_call);
}

object vector_fold_proc;

object* vector_fold(object* args, object* cont) {
	object* function;
	object* initial;
	object* vec;
	object* trace;
	delist_4(args, &function, &initial, &vec, &trace);
	
	object* next_args = alloc_list_3(function, initial, trace);
	object* next_call = alloc_call(&fold_as_list_proc, next_args, cont);
	
	object* list_args = alloc_list_1(vec);
	object* list_call = alloc_call(&vector_to_list_proc, list_args, alloc_cont(next_call));
	
	return perform_call(list_call);
}

object print_vector_proc;

object* print_vector(object* args, object* cont) {	
	printf("(vector");

	object* end_call = alloc_call(&print_sequence_end_proc, empty_list(), cont);
	object* end_cont = alloc_discarding_cont(end_call);

	object* print_call = alloc_call(&print_sequence_element_proc, args, end_cont);
	
	return perform_call(print_call);
}

void init_vector_procedures(void) {
	add_syntax("vector", syntax_vector, context_value, &vector);
	init_primitive(&vector_to_list, &vector_to_list_proc);
	init_primitive(&list_to_vector, &list_to_vector_proc);
	
	add_first_procedure(type_vector_iterator, &vector_first);
	init_primitive(&vector_rest, &vector_rest_proc);
	add_rest_procedure(type_vector_iterator, &vector_rest_proc);
	
	init_primitive(&vector_map, &vector_map_proc);
	init_primitive(&vector_list_map, &vector_list_map_proc);
	add_map_procedure(type_vector_iterator, &vector_map_proc);
	
	init_primitive(&vector_filter, &vector_filter_proc);
	init_primitive(&vector_list_filter, &vector_list_filter_proc);
	add_filter_procedure(type_vector_iterator, &vector_filter_proc);
	
	init_primitive(&vector_fold, &vector_fold_proc);
	add_fold_procedure(type_vector_iterator, &vector_fold_proc);
		
	init_primitive(&print_vector, &print_vector_proc);
	add_print_procedure(type_vector_iterator, &print_vector_proc);
}
