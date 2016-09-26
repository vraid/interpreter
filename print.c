#include <stdio.h>
#include "print.h"
#include "data-structures.h"
#include "sequences.h"
#include "global-variables.h"
#include "call.h"
#include "delist.h"
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
		object iterator;
		object* next_iter = next_iterator(&iterator, seq);
		object next_args[1];
		init_list_1(next_args, next_iter);
		object next_call;
		init_call(&next_call, &print_sequence_element_proc, next_args, cont);
		object next_cont;
		init_discarding_cont(&next_cont, &next_call);
		object print_args[1];
		init_list_1(print_args, sequence_first(seq));
		object call;
		init_call(&call, &print_proc, print_args, &next_cont);
		return perform_call(&call);
}

object* print_sequence_element(object* args, object* cont) {
	return print_sequence_element_base(0, args, cont);
}

object* print_first_sequence_element(object* args, object* cont) {
	return print_sequence_element_base(1, args, cont);
}

object* print_sequence_end(object* args, object* cont) {
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
		object end_call;
		init_call(&end_call, &print_sequence_end_proc, args, cont);
		object end_cont;
		init_discarding_cont(&end_cont, &end_call);
		object call;
		init_call(&call, &print_first_sequence_element_proc, args, &end_cont);
		return perform_call(&call);
	}	
}

object* print_vector(object* args, object* cont) {	
	printf("#");

	object print_call;
	init_call(&print_call, &print_sequence_proc, args, cont);
	
	return perform_call(&print_call);
}

object print_stream_element_proc;

object* print_stream_element_base(char is_first, object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	
	if (is_empty_stream(stream)) {
		return call_discarding_cont(cont);
	}
	else
		if (!is_first) printf(" ");
		
		object next_args[1];
		init_list_1(next_args, stream);
		object next_call;
		init_call(&next_call, &print_stream_element_proc, next_args, cont);
		object next_cont;
		init_discarding_cont(&next_cont, &next_call);
		
		object print_args[1];
		init_list_1(print_args, stream_first(stream));
		object call;
		init_call(&call, &print_proc, print_args, &next_cont);
		return perform_call(&call);
}

object* print_stream_element(object* args, object* cont) {
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
			object print_args[1];
			init_list_1(print_args, rest);
			return print_stream_element_base(0, print_args, cont);
		}
	}
}

object print_first_stream_element_proc;

object* print_first_stream_element(object* args, object* cont) {
	return print_stream_element_base(1, args, cont);
}

object* print_stream(object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	printf("#stream(");
	
	if (is_empty_stream(stream)) {
		printf(")");
		return call_discarding_cont(cont);
	}
	else {
		object end_call;
		init_call(&end_call, &print_sequence_end_proc, empty_list(), cont);
		object end_cont;
		init_discarding_cont(&end_cont, &end_call);
		
		object first_args[1];
		init_list_1(first_args, stream);
		object first_call;
		init_call(&first_call, &print_first_stream_element_proc, first_args, &end_cont);
		
		return perform_call(&first_call);
	}
}

object print_struct_proc;

object* print_struct(object* args, object* cont) {
	object* st;
	delist_1(args, &st);
	object* type = struct_instance_type(st);
	printf("#struct(%s ", string_value(symbol_name(struct_definition_name(type))));
	
	if (is_empty_list(struct_definition_fields(type))) {
		printf(")");
		return call_discarding_cont(cont);
	}
	else {
		object end_call;
		init_call(&end_call, &print_sequence_end_proc, empty_list(), cont);
		object end_cont;
		init_discarding_cont(&end_cont, &end_call);
		
		object print_args[1];
		init_list_1(print_args, struct_instance_data(st));
		object call;
		init_call(&call, &print_first_sequence_element_proc, print_args, &end_cont);
		return perform_call(&call);
	}
}

object print_integer_digits_proc;

object* print_integer_digits(object* args, object* cont) {
	object* digits;
	delist_1(args, &digits);
	
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
	
	if ((integer_sign(num) == -1) && !is_zero_integer(num)) {
		printf("-");
	}
	
	object print_call;
	init_call(&print_call, &print_integer_digits_proc, empty_list(), cont);
	object print_cont;
	init_cont(&print_cont, &print_call);
	
	object call;
	init_call(&call, &integer_to_decimal_proc, args, &print_cont);
	
	return perform_call(&call);
}

object print_fraction_denominator_proc;

object* print_fraction_denominator(object* args, object* cont) {
	object* denom;
	delist_1(args, &denom);
	
	if (is_one_integer(denom)) {
		return call_discarding_cont(cont);
	}
	else {
		printf("/");
		object print_call;
		init_call(&print_call, &print_integer_proc, args, cont);
		
		return perform_call(&print_call);
	}
}

object print_fraction_proc;

object* print_fraction(object* args, object* cont) {
	object* fraction;
	delist_1(args, &fraction);
	
	object* num = fraction_numerator(fraction);
	object* denom = fraction_denominator(fraction);
	
	object denom_args[1];
	init_list_1(denom_args, denom);
	object denom_call;
	init_call(&denom_call, &print_fraction_denominator_proc, denom_args, cont);
	object denom_cont;
	init_discarding_cont(&denom_cont, &denom_call);
	
	object num_args[1];
	init_list_1(num_args, num);
	object num_call;
	init_call(&num_call, &print_integer_proc, num_args, &denom_cont);
	
	return perform_call(&num_call);
}

object* print_newline(object* args, object* cont) {
	printf("\n");
	return call_discarding_cont(cont);
}

object* print_value(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	switch (obj->type) {
		case type_none:
			printf("undefined");
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
			return print_integer(args, cont);
			break;
		case type_fraction:
			return print_fraction(args, cont);
			break;
		case type_struct_instance:
			return print_struct(args, cont);
			break;
		case type_list:
			return print_sequence(args, cont);
			break;
		case type_stream:
			return print_stream(args, cont);
			break;
		case type_vector_iterator:
			return print_vector(args, cont);
			break;
		case type_function:
			printf("function ");
			object ls;
			init_list_1(&ls, function_parameters(obj));
			return print_sequence(&ls, cont);
			break;
		case type_primitive_procedure:
			printf("function: primitive");
			break;
		case type_syntax:
			printf("syntax");
			break;
		case type_delay:
			printf("delay");
			break;
		default:
			fprintf(stderr, "unknown type");
	}
	
	return call_discarding_cont(cont);
}

void init_print_procedures(void) {
	init_primitive(&print_value, &print_proc);
	init_primitive(&print_newline, &print_newline_proc);
	init_primitive(&print_sequence_element, &print_sequence_element_proc);
	init_primitive(&print_first_sequence_element, &print_first_sequence_element_proc);
	init_primitive(&print_sequence, &print_sequence_proc);
	init_primitive(&print_sequence_end, &print_sequence_end_proc);
	
	init_primitive(&print_stream_element, &print_stream_element_proc);
	init_primitive(&print_first_stream_element, &print_first_stream_element_proc);
	
	init_primitive(&print_struct, &print_struct_proc);
	init_primitive(&print_integer, &print_integer_proc);
	init_primitive(&print_integer_digits, &print_integer_digits_proc);
	
	init_primitive(&print_fraction, &print_fraction_proc);
	init_primitive(&print_fraction_denominator, &print_fraction_denominator_proc);
}
