#include <stdio.h>
#include "print.h"
#include "data-structures.h"
#include "global-variables.h"
#include "call.h"
#include "delist.h"
#include "object-init.h"

object print_list_element_proc;
object print_first_list_element_proc;
object print_list_proc;
object print_list_end_proc;

object* print_list_element_base(char is_first, object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	if (is_empty_list(ls)) {
		return call_discarding_cont(cont);
	}
	else
		if (!is_first) {
			printf(" ");
		}
		object ls2[1];
		init_list_1(ls2, list_rest(ls));
		object next_call;
		init_call(&next_call, &print_list_element_proc, ls2, cont);
		object next_cont;
		init_discarding_cont(&next_cont, &next_call);
		object ls3[1];
		init_list_1(ls3, list_first(ls));
		object call;
		init_call(&call, &print_proc, ls3, &next_cont);
		return perform_call(&call);
}

object* print_list_element(object* args, object* cont) {
	return print_list_element_base(0, args, cont);
}

object* print_first_list_element(object* args, object* cont) {
	return print_list_element_base(1, args, cont);
}

object* print_list_end(object* args, object* cont) {
	printf(")");
	return call_discarding_cont(cont);
}

object* print_list(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	if (is_empty_list(ls)) {
		printf("()");
		return call_discarding_cont(cont);
	}
	else {
		printf("(");
		object end_call;
		init_call(&end_call, &print_list_end_proc, args, cont);
		object end_cont;
		init_discarding_cont(&end_cont, &end_call);
		object call;
		init_call(&call, &print_first_list_element_proc, args, &end_cont);
		return perform_call(&call);
	}	
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
		case type_number:
			printf("%ld", number_value(obj));
			break;
		case type_list:
			return print_list(args, cont);
			break;
		case type_function:
			printf("function ");
			break;
		default:
			fprintf(stderr, "unknown type");
	}
	
	return call_discarding_cont(cont);
}

void init_print_procedures(void) {
	init_primitive_procedure(&print_proc, &print_value);
	init_primitive_procedure(&print_newline_proc, &print_newline);
	init_primitive_procedure(&print_list_element_proc, &print_list_element);
	init_primitive_procedure(&print_first_list_element_proc, &print_first_list_element);
	init_primitive_procedure(&print_list_proc, &print_list);
	init_primitive_procedure(&print_list_end_proc, &print_list_end);
}
