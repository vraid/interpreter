#include "sequences.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "base-util.h"
#include "list-util.h"
#include "integers.h"
#include "vectors.h"
#include "streams.h"
#include "delist.h"
#include "call.h"

object* first(object* args, object* cont) {
	object* seq;
	delist_1(args, &seq);
	
	if (!is_sequence(seq)) {
		return throw_error_string(cont, "first on non-sequence");
	}
	else {
		return call_cont(cont, sequence_first(seq));
	}
}

object* rest(object* args, object* cont) {
	object* seq;
	delist_1(args, &seq);
	
	if (!is_sequence(seq)) {
		return throw_error_string(cont, "rest on non-sequence");
	}
	else if (is_empty_sequence(seq)) {
		return throw_error_string(cont, "rest on empty sequence");
	}
	else if (is_stream(seq)) {
		object* eval_args = alloc_list_1(seq);
		object* eval_call = alloc_call(&eval_stream_rest_proc, eval_args, cont);
		
		return perform_call(eval_call);
	}
	else {
		object next_iter;
		object* next = next_iterator(&next_iter, seq);
		return call_cont(cont, next);
	}
}


object take_single_proc;
object take_rest_proc;

object* take_single(object* args, object* cont) {
	object* sequence;
	object* last;
	object* count;
	delist_3(args, &sequence, &last, &count);
	
	object* rest_args = alloc_list_2(count, sequence);
	object* rest_call = alloc_call(&take_rest_proc, rest_args, cont);
	object* rest_cont = alloc_cont(rest_call);
	
	object* add_args = alloc_list_2(sequence_first(sequence), last);
	object* add_call = alloc_call(&add_to_list_proc, add_args, rest_cont);
	
	return perform_call(add_call);
}

object take_rest_second_proc;

object* take_rest_second(object* args, object* cont) {
	object* count;
	object* last;
	object* sequence;
	delist_3(args, &count, &last, &sequence);
	
	if (integer_is_zero(count) || is_empty_sequence(sequence)) {
		return call_discarding_cont(cont);
	}
	else {
		object* take_args = alloc_list_2(last, count);
		object* take_call = alloc_call(&take_single_proc, take_args, cont);
		object* take_cont = alloc_cont(take_call);
		
		object* eval_args = alloc_list_1(sequence);
		object* eval_call = alloc_call(&rest_proc, eval_args, take_cont);
		
		return perform_call(eval_call);
	}
}

object* take_rest(object* args, object* cont) {
	object* last;
	object* count;
	object* sequence;
	delist_3(args, &last, &count, &sequence);
	
	object* rest_args = alloc_list_2(last, sequence);
	object* rest_call = alloc_call(&take_rest_second_proc, rest_args, cont);
	object* rest_cont = alloc_cont(rest_call);
	
	object* dec_args = alloc_list_1(count);
	object* dec_call = alloc_call(&integer_subtract_one_proc, dec_args, rest_cont);
	
	return perform_call(dec_call);
}

object take_first_proc;

object* take_first(object* args, object* cont) {
	object* count;
	object* sequence;
	delist_2(args, &count, &sequence);
	
	object* take_args = alloc_list_2(count, sequence);
	object* list_args = alloc_list_3(sequence_first(sequence), &take_rest_proc, take_args);
	object* list_call = alloc_call(&make_list_proc, list_args, cont);
	
	return perform_call(list_call);
}

object* take(object* args, object* cont) {
	object* count;
	object* sequence;
	delist_2(args, &count, &sequence);
	
	if (integer_is_zero(count) || is_empty_sequence(sequence)) {
		return call_cont(cont, empty_list());
	}
	else {
		object* call = alloc_call(&take_first_proc, args, cont);
		
		return perform_call(call);
	}
}

object drop_single_proc;
object drop_single_second_proc;

object* drop_single_second(object* args, object* cont) {
	object* count;
	object* sequence;
	delist_2(args, &count, &sequence);
	
	if (integer_is_zero(count) || is_empty_sequence(sequence)) {
		return call_cont(cont, sequence);
	}
	else {
		object* drop_args = alloc_list_1(count);
		object* drop_call = alloc_call(&drop_single_proc, drop_args, cont);
		object* drop_cont = alloc_cont(drop_call);
		
		object* eval_args = alloc_list_1(sequence);
		object* eval_call = alloc_call(&rest_proc, eval_args, drop_cont);
		
		return perform_call(eval_call);
	}
}

object* drop_single(object* args, object* cont) {
	object* sequence;
	object* count;
	delist_2(args, &sequence, &count);
	
	object* drop_args = alloc_list_1(sequence);
	object* drop_call = alloc_call(&drop_single_second_proc, drop_args, cont);
	object* drop_cont = alloc_cont(drop_call);
	
	object* dec_args = alloc_list_1(count);
	object* dec_call = alloc_call(&integer_subtract_one_proc, dec_args, drop_cont);
	
	return perform_call(dec_call);
}

object* drop(object* args, object* cont) {
	object* count;
	object* sequence;
	delist_2(args, &count, &sequence);
	
	object* call_args = alloc_list_2(sequence, count);
	object* call = alloc_call(&drop_single_proc, call_args, cont);
	
	return perform_call(call);
}

int sequence_length(object* obj) {
	switch(obj->type) {
		case type_list: return list_length(obj);
		case type_vector_iterator: return vector_iterator_length(obj);
		default:
			fprintf(stderr, "invalid sequence type: %s\n", object_type_name(obj));
			exit(0);
	}
}

object* sequence_first(object* obj) {
	switch (obj->type) {
		case type_list: return list_first(obj);
		case type_stream: return stream_first(obj);
		case type_vector_iterator: return vector_ref(obj, 0);
		default:
			fprintf(stderr, "invalid sequence type: %s\n", object_type_name(obj));
			exit(0);
	}
}

object* sequence_rest(object* iter, object* obj) {
	return next_iterator(iter, obj);
}

object* list_to_sequence_proc(object_type type) {
	switch (type) {
		case type_list: return &identity_proc;
		case type_vector_iterator: return &list_to_vector_proc;
		default:
			fprintf(stderr, "invalid sequence type: %s\n", type_name(type));
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
			fprintf(stderr, "invalid sequence type: %s\n", object_type_name(obj));
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
			fprintf(stderr, "invalid iterator type: %s\n", object_type_name(current));
			return no_object();
	}
}

void init_sequence_procedures(void) {
	init_primitive(&first, &first_proc);
	init_primitive(&rest, &rest_proc);
	init_primitive(&take, &take_proc);
	init_primitive(&take_first, &take_first_proc);
	init_primitive(&take_single, &take_single_proc);
	init_primitive(&take_rest, &take_rest_proc);
	init_primitive(&take_rest_second, &take_rest_second_proc);
	init_primitive(&drop, &drop_proc);
	init_primitive(&drop_single, &drop_single_proc);
	init_primitive(&drop_single_second, &drop_single_second_proc);
}
