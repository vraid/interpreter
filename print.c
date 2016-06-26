#include <stdio.h>
#include "print.h"
#include "data-structures.h"
#include "sequences.h"
#include "global-variables.h"
#include "call.h"
#include "delist.h"
#include "object-init.h"
#include "vector-util.h"

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

object print_first_sequence_element_proc;
object print_sequence_proc;
object print_sequence_end_proc;

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
		case type_number:
			printf("%ld", number_value(obj));
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
		case type_delay:
			printf("delay");
			break;
		default:
			fprintf(stderr, "unknown type");
	}
	
	return call_discarding_cont(cont);
}

void init_print_procedures(void) {
	init_primitive_procedure(&print_proc, &print_value);
	init_primitive_procedure(&print_newline_proc, &print_newline);
	init_primitive_procedure(&print_sequence_element_proc, &print_sequence_element);
	init_primitive_procedure(&print_first_sequence_element_proc, &print_first_sequence_element);
	init_primitive_procedure(&print_sequence_proc, &print_sequence);
	init_primitive_procedure(&print_sequence_end_proc, &print_sequence_end);
	
	init_primitive_procedure(&print_stream_element_proc, &print_stream_element);
	init_primitive_procedure(&print_first_stream_element_proc, &print_first_stream_element);
}
