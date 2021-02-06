#include "sequences.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "base-util.h"
#include "list-util.h"
#include "string-util.h"
#include "integers.h"
#include "vectors.h"
#include "streams.h"
#include "delist.h"
#include "call.h"

sequence_first_func* sequence_first_procedure[type_count];
object* sequence_rest_procedure[type_count];

object* sequence_first(object* seq) {
	return sequence_first_procedure[seq->type](seq);
}

void add_first_procedure(object_type type, sequence_first_func* proc) {
	sequence_first_procedure[type] = proc;
}

void add_rest_procedure(object_type type, object* proc) {
	sequence_rest_procedure[type] = proc;
}

object* first_undefined_proc(object* seq) {
	suppress_warning(seq);
	return no_object();
}

char is_undefined_first(sequence_first_func* proc) {
	return proc == &first_undefined_proc;
}

object rest_undefined_proc;

object* rest_undefined(object* args, object* cont) {
	object* sequence;
	delist_1(args, &sequence);
	
	object* str = alloc_string_append_2(alloc_string("rest not defined on type: "), alloc_string(object_type_name(sequence)));
	return throw_error(cont, str);
}

object* first(object* args, object* cont) {
	object* seq;
	delist_1(args, &seq);
	
	if (is_empty_sequence(seq)) {
		return throw_error_string(cont, "first on empty sequence");
	}
	else {
		sequence_first_func* proc = sequence_first_procedure[seq->type];
		if (is_undefined_first(proc)) {
			object* str = alloc_string_append_2(alloc_string("first not defined on type: "), alloc_string(object_type_name(seq)));
			return throw_error(cont, str);
		}
		else {
			return call_cont(cont, proc(seq));
		}
	}
}

object* rest(object* args, object* cont) {
	object* seq;
	delist_1(args, &seq);
	
	if (is_empty_sequence(seq)) {
		return throw_error_string(cont, "rest on empty sequence");
	}
	else {
		object* call = alloc_call(sequence_rest_procedure[seq->type], args, cont);
		return perform_call(call);
	}
}

object take_drop_decrement_proc;
object take_drop_proc;

object* take_drop_decrement(object* args, object* cont) {
	object* rest;
	object* taken;
	object* n;
	delist_3(args, &rest, &taken, &n);
	
	object* next_call = alloc_call(&take_drop_proc, alloc_list_2(rest, taken), cont);
	
	object* dec_call = alloc_call(&integer_subtract_one_proc, alloc_list_1(n), alloc_cont(next_call));
	return perform_call(dec_call);
}

object* take_drop(object* args, object* cont) {
	object* n;
	object* rest;
	object* taken;
	delist_3(args, &n, &rest, &taken);
	
	if (integer_is_negative(n)) {
		return throw_error_string(cont, "cannot take/drop negative amount");
	}
	else if (integer_is_zero(n) || is_empty_sequence(rest)) {
		return call_cont(cont, alloc_list_2(taken, rest));
	}
	else {
		taken = alloc_list_cell(sequence_first(rest), taken);
		object* next_args = alloc_list_2(taken, n);
		object* next_call = alloc_call(&take_drop_decrement_proc, next_args, cont);
		
		object* rest_call = alloc_call(&rest_proc, alloc_list_1(rest), alloc_cont(next_call));
		return perform_call(rest_call);
	}
}

object take_return_proc;

object* take_return(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	object* taken;
	object* rest;
	delist_2(ls, &taken, &rest);
	
	return call_cont(cont, taken);
}

object take_proc;

object* take(object* args, object* cont) {
	object* n;
	object* seq;
	delist_2(args, &n, &seq);
	
	object* reverse_call = alloc_call(&reverse_list_proc, empty_list(), cont);
	object* return_call = alloc_call(&take_return_proc, empty_list(), alloc_cont(reverse_call));
	object* call = alloc_call(&take_drop_proc, alloc_list_3(n, seq, empty_list()), alloc_cont(return_call));
	return perform_call(call);
}

object drop_return_proc;

object* drop_return(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	object* taken;
	object* rest;
	delist_2(ls, &taken, &rest);
	
	return call_cont(cont, rest);
}

object drop_proc;

object* drop(object* args, object* cont) {
	object* n;
	object* seq;
	delist_2(args, &n, &seq);
	
	object* return_call = alloc_call(&drop_return_proc, empty_list(), cont);
	object* call = alloc_call(&take_drop_proc, alloc_list_3(n, seq, empty_list()), alloc_cont(return_call));
	return perform_call(call);
}

void init_sequence_procedures(void) {
	init_primitive(&rest_undefined, &rest_undefined_proc);
	
	init_primitive(&first, &first_proc);
	init_primitive(&rest, &rest_proc);
	
	init_primitive(&take_drop, &take_drop_proc);
	init_primitive(&take_drop_decrement, &take_drop_decrement_proc);
	
	init_primitive(&take, &take_proc);
	init_primitive(&take_return, &take_return_proc);
	
	init_primitive(&drop, &drop_proc);
	init_primitive(&drop_return, &drop_return_proc);
	
	for (object_type k = type_none; k < type_count; k++) {
		add_first_procedure(k, &first_undefined_proc);
		add_rest_procedure(k, &rest_undefined_proc);
	}
}
