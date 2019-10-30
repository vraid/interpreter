#include "print.h"

#include <stdlib.h>
#include <stdio.h>
#include "data-structures.h"
#include "sequences.h"
#include "global-variables.h"
#include "call.h"
#include "delist.h"
#include "base-util.h"
#include "list-util.h"
#include "object-init.h"
#include "vectors.h"
#include "integers.h"

object print_sequence_element_proc;
object print_first_sequence_element_proc;
object print_sequence_proc;
object print_sequence_end_proc;

object* print_sequence_element_base(char is_first, object* args, object* cont) {
	object* seq;
	delist_1(args, &seq);
	
	if (is_empty_sequence(seq)) {
		return call_discarding_cont(cont);
	}
	else
		if (!is_first) printf(" ");
		object* next_iter = alloc_next_iterator(seq);
		object* next_args = alloc_list_1(next_iter);
		object* next_call = alloc_call(&print_sequence_element_proc, next_args, cont);
		object* next_cont = alloc_discarding_cont(next_call);
		object* print_args = alloc_list_1(sequence_first(seq));
		object* call = alloc_call(&print_value_proc, print_args, next_cont);
		return perform_call(call);
}

object* print_sequence_element(object* args, object* cont) {
	return print_sequence_element_base(0, args, cont);
}

object* print_first_sequence_element(object* args, object* cont) {
	return print_sequence_element_base(1, args, cont);
}

object* print_sequence_end(object* args, object* cont) {
	suppress_warning(args);
	printf(")");
	return call_discarding_cont(cont);
}

object* print_sequence(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	if (is_empty_sequence(ls)) {
		printf("()");
		return call_discarding_cont(cont);
	}
	else {
		printf("(");
		object* end_call = alloc_call(&print_sequence_end_proc, args, cont);
		object* end_cont = alloc_discarding_cont(end_call);
		object* call = alloc_call(&print_first_sequence_element_proc, args, end_cont);
		return perform_call(call);
	}	
}

object* print_vector(object* args, object* cont) {	
	printf("(vector");

	object* end_call = alloc_call(&print_sequence_end_proc, empty_list(), cont);
	object* end_cont = alloc_discarding_cont(end_call);

	object* print_call = alloc_call(&print_sequence_element_proc, args, end_cont);
	
	return perform_call(print_call);
}

object print_stream_element_proc;
object print_stream_rest_proc;

object* print_stream_element(object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	
	if (is_empty_stream(stream)) {
		return call_discarding_cont(cont);
	}
	else
		printf(" ");
		
		object* next_args = alloc_list_1(stream);
		object* next_call = alloc_call(&print_stream_rest_proc, next_args, cont);
		object* next_cont = alloc_discarding_cont(next_call);
		
		object* print_args = alloc_list_1(stream_first(stream));
		object* call = alloc_call(&print_value_proc, print_args, next_cont);
		return perform_call(call);
}

object* print_stream_rest(object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	
	object* delay = stream_rest(stream);
	
	if (!delay_evaluated(delay)) {
		printf(" ..");
		return call_discarding_cont(cont);
	}
	else {
		object* rest = delay_value(delay);
		if (is_empty_stream(rest)) {
			return call_discarding_cont(cont);
		}
		else {
			object* print_args = alloc_list_1(rest);
			object* call = alloc_call(&print_stream_element_proc, print_args, cont);
			
			return perform_call(call);
		}
	}
}

object* print_stream(object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	printf("(stream");
	
	object* end_call = alloc_call(&print_sequence_end_proc, empty_list(), cont);
	object* end_cont = alloc_discarding_cont(end_call);
	
	object* first_args = alloc_list_1(stream);
	object* first_call = alloc_call(&print_stream_element_proc, first_args, end_cont);
	
	return perform_call(first_call);
}

object print_struct_proc;

object* print_struct(object* args, object* cont) {
	object* st;
	delist_1(args, &st);
	object* type = struct_instance_type(st);
	printf("(struct:%s", string_value(symbol_name(struct_definition_name(type))));
	
	object* end_call = alloc_call(&print_sequence_end_proc, empty_list(), cont);
	object* end_cont = alloc_discarding_cont(end_call);
	
	object* print_args = alloc_list_1(struct_instance_data(st));
	object* call = alloc_call(&print_sequence_element_proc, print_args, end_cont);
	return perform_call(call);
}

object print_integer_digits_proc;

object* print_integer_digits(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	object* digits = integer_digits(num);
	
	while (!is_empty_list(digits)) {
		printf("%lld", fixnum_value(list_first(digits)));
		digits = list_rest(digits);
	}
	
	return call_discarding_cont(cont);
}

object print_integer_proc;

object* print_integer(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	if (integer_sign(num) == -1) {
		printf("-");
	}
	
	object* print_call = alloc_call(&print_integer_digits_proc, empty_list(), cont);
	object* call = alloc_call(&integer_to_decimal_proc, args, alloc_cont(print_call));
	
	return perform_call(call);
}

object print_fraction_denominator_proc;

object* print_fraction_denominator(object* args, object* cont) {
	object* denom;
	delist_1(args, &denom);
	
	if (integer_is_one(denom)) {
		return call_discarding_cont(cont);
	}
	else {
		printf("/");
		object* print_call = alloc_call(&print_integer_proc, args, cont);
		
		return perform_call(print_call);
	}
}

object print_fraction_proc;

object* print_fraction(object* args, object* cont) {
	object* fraction;
	delist_1(args, &fraction);
	
	object* num = fraction_numerator(fraction);
	object* denom = fraction_denominator(fraction);
	
	object* denom_args = alloc_list_1(denom);
	object* denom_call = alloc_call(&print_fraction_denominator_proc, denom_args, cont);
	object* denom_cont = alloc_discarding_cont(denom_call);
	
	object* num_args = alloc_list_1(num);
	object* num_call = alloc_call(&print_integer_proc, num_args, denom_cont);
	
	return perform_call(num_call);
}

object print_complex_proc;

object* print_complex(object* args, object* cont) {
	object* complex;
	delist_1(args, &complex);
	
	printf("(complex ");
	
	object* end_call = alloc_call(&print_sequence_end_proc, empty_list(), cont);
	object* end_cont = alloc_discarding_cont(end_call);
	
	object* ls = alloc_list_2(complex_real_part(complex), complex_imag_part(complex));
	object* print_args = alloc_list_1(ls);
	object* call = alloc_call(&print_first_sequence_element_proc, print_args, end_cont);
	return perform_call(call);
}

object trace_to_origin_proc;

object* trace_to_origin(object* args, object* cont) {
	object* result;
	object* trace;
	delist_2(args, &result, &trace);
	
	if (is_empty_list(trace)) {
		object* call_args = alloc_list_1(result);
		object* call = alloc_call(&reverse_list_proc, call_args, cont);
		
		return perform_call(call);
	}
	else {
		object* first = list_first(trace);
		object* res = alloc_list_cell(syntax_object_origin(first), result);
		
		object* call_args = alloc_list_2(res, list_rest(trace));
		object* call = alloc_call(&trace_to_origin_proc, call_args, cont);
		
		return perform_call(call);
	}
}

object print_internal_position_proc;

object* print_internal_position(object* args, object* cont) {
	object* pos;
	delist_1(args, &pos);
	
	printf("(pos %i)", internal_position_n(pos));
	
	return call_discarding_cont(cont);
}

object print_error_trace_proc;

object* print_error_trace(object* args, object* cont) {
	object* trace;
	delist_1(args, &trace);
	
	if (is_empty_list(trace)) {
		return call_discarding_cont(cont);
	}
	else {
		printf("\nin ");
		
		object* print_call = alloc_call(&print_sequence_proc, empty_list(), cont);
		
		object* origin_args = alloc_list_2(empty_list(), trace);
		object* origin_call = alloc_call(&trace_to_origin_proc, origin_args, alloc_cont(print_call));
		
		return perform_call(origin_call);
	}
}

object print_internal_error_proc;

object* print_internal_error(object* args, object* cont) {
	object* e;
	delist_1(args, &e);
	
	object* trace = internal_error_trace(e);
	trace = is_false(trace) ? empty_list() : trace;
	
	object* trace_args = alloc_list_1(trace);
	object* trace_call = alloc_call(&print_error_trace_proc, trace_args, cont);
	object* trace_cont = alloc_discarding_cont(trace_call);
	
	object* print_args = alloc_list_1(internal_error_message(e));
	
	return print_value(print_args, trace_cont);
}

object* print_newline(object* args, object* cont) {
	suppress_warning(args);
	printf("\n");
	return call_discarding_cont(cont);
}

object* print_value(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	obj = desyntax(obj);
	object* print_args = alloc_list_1(obj);
	
	switch (obj->type) {
		case type_nothing:
			break;
		case type_string:
			printf("\"%s\"", string_value(obj));
			break;
		case type_symbol:
			printf("%s", string_value(symbol_name(obj)));
			break;
		case type_boolean:
			if (is_false(obj)) {
				printf("#f");
			}
			else if (is_true(obj)) {
				printf("#t");
			}
			else {
				printf("error\n");
				fprintf(stderr, "erroneous boolean");
			}
			break;
		case type_fixnum:
			printf("%lld", fixnum_value(obj));
			break;
		case type_integer:
			return print_integer(print_args, cont);
			break;
		case type_fraction:
			return print_fraction(print_args, cont);
			break;
		case type_complex:
			return print_complex(print_args, cont);
			break;
		case type_struct_instance:
			return print_struct(print_args, cont);
			break;
		case type_list:
			return print_sequence(print_args, cont);
			break;
		case type_stream:
			return print_stream(print_args, cont);
			break;
		case type_vector_iterator:
			return print_vector(print_args, cont);
			break;
		case type_delay:
			printf("(delay:)");
			break;
		case type_function:
			printf("function ");
			object* ls = alloc_list_1(function_parameters(obj));
			return print_sequence(ls, cont);
			break;
		case type_syntax_procedure:
			printf("syntax:%s", syntax_names[syntax_procedure_id(obj)]);
			break;
		case type_internal_position:
			return print_internal_position(print_args, cont);
			break;
		case type_internal_error:
			return print_internal_error(print_args, cont);
			break;
		default:
			fprintf(stderr, "%s", object_type_name(obj));
	}
	
	return call_discarding_cont(cont);
}

object* print_entry(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	if (is_syntax_object(obj)) {
		object* pos = syntax_object_position(obj);
		printf("(syntax:%i ", internal_position_n(pos));
		
		object* end_call = alloc_call(&print_sequence_end_proc, empty_list(), cont);
		object* end_cont = alloc_discarding_cont(end_call);
		
		object* print_args = alloc_list_1(syntax_object_syntax(obj));
		return print_value(print_args, end_cont);
	}
	else {
		return print_value(args, cont);
	}
}

void init_print_procedures(void) {
	init_primitive(&print_entry, &print_entry_proc);
	init_primitive(&print_value, &print_value_proc);
	init_primitive(&print_newline, &print_newline_proc);
	init_primitive(&print_sequence_element, &print_sequence_element_proc);
	init_primitive(&print_first_sequence_element, &print_first_sequence_element_proc);
	init_primitive(&print_sequence, &print_sequence_proc);
	init_primitive(&print_sequence_end, &print_sequence_end_proc);
	
	init_primitive(&print_stream_element, &print_stream_element_proc);
	init_primitive(&print_stream_rest, &print_stream_rest_proc);
	
	init_primitive(&print_struct, &print_struct_proc);
	init_primitive(&print_integer, &print_integer_proc);
	init_primitive(&print_integer_digits, &print_integer_digits_proc);
	
	init_primitive(&print_fraction, &print_fraction_proc);
	init_primitive(&print_fraction_denominator, &print_fraction_denominator_proc);

	init_primitive(&print_complex, &print_complex_proc);
	init_primitive(&trace_to_origin, &trace_to_origin_proc);
	init_primitive(&print_internal_position, &print_internal_position_proc);
	init_primitive(&print_internal_error, &print_internal_error_proc);
	init_primitive(&print_error_trace, &print_error_trace_proc);
}
