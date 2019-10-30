#include "equivalence.h"

#include <stdlib.h>
#include "data-structures.h"
#include "object-init.h"
#include "global-variables.h"
#include "call.h"
#include "delist.h"
#include "vectors.h"
#include "base-util.h"
#include "syntax-base.h"

object* equivalence[type_count];

object no_equivalence_proc;

object* no_equivalence(object* args, object* cont) {
	suppress_warning(args);
	return call_cont(cont, false());
}

object string_equivalence_proc;

object* string_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (string_length(a) != string_length(b)) {
		return call_cont(cont, false());
	}
	else {
		int n;
		char eq = 1;
		int length = string_length(a);
		char* aval = string_value(a);
		char* bval = string_value(b); 
		for (n = 0; n < length; n++) {
			if (aval[n] != bval[n]) {
				eq = 0;
				break;
			}
		}
		return call_cont(cont, boolean(eq));
	}
}

object list_element_equivalence_proc;

object* list_element_equivalence(object* args, object* cont) {
	object* eq;
	object* a;
	object* b;
	delist_3(args, &eq, &a, &b);
	
	char a_empty = is_empty_list(a);
	char b_empty = is_empty_list(b);
	
	if (is_false(eq)) {
		return call_cont(cont, false());
	}
	else if (a_empty && b_empty) {
		return call_cont(cont, eq);
	}
	else if (a_empty || b_empty) {
		return call_cont(cont, false());
	}
	else {
		object* list_call = alloc_call(&list_element_equivalence_proc, alloc_list_2(list_rest(a), list_rest(b)), cont);
		object* eq_call = alloc_call(&is_equivalent_proc, alloc_list_2(list_first(a), list_first(b)), alloc_cont(list_call));
		return perform_call(eq_call);
	}
}

object list_equivalence_proc;

object* list_equivalence(object* args, object* cont) {
	object* call = alloc_call(&list_element_equivalence_proc, alloc_list_cell(true(), args), cont);
	return perform_call(call);
}


object stream_equivalence_proc;

object* stream_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* avals = alloc_list_2(stream_first(a), stream_rest(a));
	object* bvals = alloc_list_2(stream_first(b), stream_rest(b));
	object* call = alloc_call(&is_equivalent_proc, alloc_list_2(avals, bvals), cont);
	return perform_call(call);
}

object vector_element_equivalence_proc;

object* vector_element_equivalence(object* args, object* cont) {
	object* eq;
	object* a;
	object* b;
	delist_3(args, &eq, &a, &b);
	
	int an = vector_iterator_index(a);
	int bn = vector_iterator_index(b);
	object* avec = vector_iterator_vector(a);
	object* bvec = vector_iterator_vector(b);
	char a_end = an == vector_length(avec);
	char b_end = bn == vector_length(bvec);
	
	if (is_false(eq)) {
		return call_cont(cont, false());
	}
	else if (a_end && b_end) {
		return call_cont(cont, eq);
	}
	else if (a_end || b_end) {
		return call_cont(cont, false());
	}
	else {
		object* next_args = alloc_list_2(alloc_vector_iterator(an+1, avec), alloc_vector_iterator(bn+1, bvec));
		object* next_call = alloc_call(&vector_element_equivalence_proc, next_args, cont);
		object* eq_args = alloc_list_2(vector_ref(a, 0), vector_ref(b, 0));
		object* eq_call = alloc_call(&is_equivalent_proc, eq_args, alloc_cont(next_call));
		return perform_call(eq_call);
	}
}

object vector_iterator_equivalence_proc;

object* vector_iterator_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	int alength = vector_iterator_length(a);
	int blength = vector_iterator_length(b);
	
	if (alength != blength) {
		return call_cont(cont, false());
	}
	else {
		return perform_call(alloc_call(&vector_element_equivalence_proc, alloc_list_cell(true(), args), cont));
	}
}

object vector_equivalence_proc;

object* vector_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* call_args = alloc_list_2(alloc_vector_iterator(0, a), alloc_vector_iterator(0, b));
	object* call = alloc_call(&vector_iterator_equivalence_proc, call_args, cont);
	return perform_call(call);
}

object fixnum_equivalence_proc;

object* fixnum_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	return call_cont(cont, boolean(fixnum_value(a) == fixnum_value(b)));
}

object integer_equivalence_proc;

object* integer_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (integer_sign(a) != integer_sign(b)) {
		return call_cont(cont, false());
	}
	else {
		object* call = alloc_call(&is_equivalent_proc, alloc_list_2(integer_digits(a), integer_digits(b)), cont);
		return perform_call(call);
	}
}

object fraction_equivalence_proc;

object* fraction_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* avals = alloc_list_2(fraction_numerator(a), fraction_denominator(a));
	object* bvals = alloc_list_2(fraction_numerator(b), fraction_denominator(b));
	object* call = alloc_call(&is_equivalent_proc, alloc_list_2(avals, bvals), cont);
	return perform_call(call);
}

object complex_equivalence_proc;

object* complex_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* avals = alloc_list_2(complex_real_part(a), complex_imag_part(a));
	object* bvals = alloc_list_2(complex_real_part(b), complex_imag_part(b));
	object* call = alloc_call(&is_equivalent_proc, alloc_list_2(avals, bvals), cont);
	return perform_call(call);
}

object struct_instance_equivalence_proc;

object* struct_instance_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (struct_instance_type(a) != struct_instance_type(b)) {
		return call_cont(cont, false());
	}
	else {
		object* call_args = alloc_list_2(struct_instance_data(a), struct_instance_data(b));
		object* call = alloc_call(&is_equivalent_proc, call_args, cont);
		return perform_call(call);
	}
}

object forced_delay_equivalence_proc;

object* forced_delay_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);

	object* call_args = alloc_list_2(delay_value(a), delay_value(b));
	object* call = alloc_call(&is_equivalent_proc, call_args, cont);
	return perform_call(call);
}

object delay_equivalence_proc;

object* delay_equivalence(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* next_call = alloc_call(&forced_delay_equivalence_proc, args, cont);
	object* discard_second = alloc_call(&discard_proc, empty_list(), alloc_discarding_cont(next_call));
	object* force_second_call = alloc_call(&eval_force_proc, alloc_list_1(b), alloc_cont(discard_second));
	object* discard_first = alloc_call(&discard_proc, empty_list(), alloc_discarding_cont(force_second_call));
	object* force_first_call = alloc_call(&eval_force_proc, alloc_list_1(a), alloc_cont(discard_first));
	return perform_call(force_first_call);
}

object* is_equivalent(object* args, object* cont) {
	object* a;
	object* b;
	delist_desyntax_2(args, &a, &b);
	
	if (a == b) {
		return call_cont(cont, true());
	}
	else if (a->type == b->type) {
		return perform_call(alloc_call(equivalence[a->type], alloc_list_2(a, b), cont));
	}
	else {
		return call_cont(cont, false());
	}
}

void init_equivalence_procedures(void) {
	init_primitive(&no_equivalence, &no_equivalence_proc);
	init_primitive(&string_equivalence, &string_equivalence_proc);
	init_primitive(&list_element_equivalence, &list_element_equivalence_proc);
	init_primitive(&list_equivalence, &list_equivalence_proc);
	init_primitive(&stream_equivalence, &stream_equivalence_proc);
	init_primitive(&vector_element_equivalence, &vector_element_equivalence_proc);
	init_primitive(&vector_iterator_equivalence, &vector_iterator_equivalence_proc);
	init_primitive(&vector_equivalence, &vector_equivalence_proc);
	init_primitive(&fixnum_equivalence, &fixnum_equivalence_proc);
	init_primitive(&integer_equivalence, &integer_equivalence_proc);
	init_primitive(&fraction_equivalence, &fraction_equivalence_proc);
	init_primitive(&complex_equivalence, &complex_equivalence_proc);
	init_primitive(&struct_instance_equivalence, &struct_instance_equivalence_proc);
	init_primitive(&forced_delay_equivalence, &forced_delay_equivalence_proc);
	init_primitive(&delay_equivalence, &delay_equivalence_proc);
	init_primitive(&is_equivalent, &is_equivalent_proc);
	
	object_type type;
	for (type = type_none; type < type_count; type++) {
		equivalence[type] = &no_equivalence_proc;
	}
	equivalence[type_string] = &string_equivalence_proc;
	equivalence[type_list] = &list_equivalence_proc;
	equivalence[type_stream] = &stream_equivalence_proc;
	equivalence[type_vector] = &vector_equivalence_proc;
	equivalence[type_vector_iterator] = &vector_iterator_equivalence_proc;
	equivalence[type_fixnum] = &fixnum_equivalence_proc;
	equivalence[type_integer] = &integer_equivalence_proc;
	equivalence[type_fraction] = &fraction_equivalence_proc;
	equivalence[type_complex] = &complex_equivalence_proc;
	equivalence[type_struct_instance] = &struct_instance_equivalence_proc;
	equivalence[type_delay] = &delay_equivalence_proc;
}
