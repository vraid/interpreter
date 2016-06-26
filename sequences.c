#include "sequences.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "base-util.h"
#include "vector-util.h"
#include "streams.h"
#include "delist.h"
#include "call.h"

object _first_proc;
object _rest_proc;

object* first_proc(void) {
	return &_first_proc;
}

object* rest_proc(void) {
	return &_rest_proc;
}

object* first(object* args, object* cont) {
	object* seq;
	delist_1(args, &seq);
	
	if (!is_sequence(seq)) {
		return throw_error(cont, "first on non-sequence");
	}
	else {
		return call_cont(cont, sequence_first(seq));
	}
}

object* rest(object* args, object* cont) {
	object* seq;
	delist_1(args, &seq);
	
	if (!is_sequence(seq)) {
		return throw_error(cont, "rest on non-sequence");
	}
	else if (is_empty_sequence(seq)) {
		return throw_error(cont, "rest on empty sequence");
	}
	else if (is_stream(seq)) {
		object eval_args[1];
		init_list_1(eval_args, seq);
		object eval_call;
		init_call(&eval_call, &eval_stream_rest_proc, eval_args, cont);
		
		return perform_call(&eval_call);
	}
	else {
		object next_iter;
		object* next = next_iterator(&next_iter, seq);
		return call_cont(cont, next);
	}
}

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
		case type_stream: return stream_first(obj);
		case type_vector_iterator: return vector_ref(obj, 0);
		default:
			fprintf(stderr, "invalid sequence type: %s\n", type_name[obj->type]);
			exit(0);
	}
}

object* sequence_rest(object* iter, object* obj) {
	return next_iterator(iter, obj);
}

object* list_to_sequence_proc(object_type type) {
	switch (type) {
		case type_list: return identity_proc();
		case type_vector_iterator: return &list_to_vector_proc;
		default:
			fprintf(stderr, "invalid sequence type: %s\n", type_name[type]);
			exit(0);
	}
}

object* first_vector_iterator(object* iter, object* obj) {
	if (vector_length(obj) == 0) {
		return end_vector_iterator();
	}
	else {
		init_vector_iterator(iter, 0, obj);
		return iter;
	}
}

object* first_iterator(object* iter, object* obj) {
	switch (obj->type) {
		case type_list:
		case type_stream:
		case type_vector_iterator: return obj;
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
		next = end_vector_iterator();
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

void init_sequence_procedures(void) {
	init_primitive_procedure(first_proc(), &first);
	init_primitive_procedure(rest_proc(), &rest);
}
