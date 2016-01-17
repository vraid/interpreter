#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "data-structures.h"
#include "allocation.h"
#include "standard-library.h"
#include "global-variables.h"
#include "environments.h"
#include "list-util.h"
#include "read.h"

object* eval(object* environment, object* exp);

object* evaluate_values(object* env, object* expr) {
	if (is_empty_list(expr)) {
		return empty_list();
	}
	else {
		object* ls = allocate_list();
		object* prev;
		object* next = ls;
		while (!is_empty_list(expr)) {
			prev = next;
			prev->data.list.first = eval(env, list_first(expr));
			expr = list_rest(expr);
		
			if (is_empty_list(expr)) {
				next = empty_list();
			}
			else {
				next = allocate_list();
			}
			prev->data.list.rest = next;
		}
		return ls;
	}
}

object* evaluate_definition(object* environment, object* exp) {
	object* binding = make_binding(list_ref(1, exp), eval(environment, list_ref(2, exp)));
	return extend_environment(environment, cons(binding, empty_list()));
}

object* quote(object* value) {
	return cons(quote_symbol(), cons(value, empty_list()));
}

object* make_lambda(object* parameters, object* body) {
	return cons(lambda_symbol(), cons(parameters, cons(body, empty_list())));
}

object* evaluate_lambda(object* environment, object* exp) {
	object* parameters = list_ref(1, exp);
	object* body = list_ref(2, exp);
	return make_function(environment, parameters, body);
}

object* evaluate_function_call(object* environment, object* function, object* values) {
	object* bindings = make_binding_list(function_parameters(function), values);
	return eval(extend_environment(function_environment(function), bindings), function_body(function));
}

void write(object* obj);

object* evaluate_map(object* environment, object* exp) {
	object* obj = list_ref(2, exp);
	object* f = eval(environment, list_ref(1, exp));
	object* values = eval(environment, obj);
	if (is_empty_list(values)) {
		return values;
	}
	else {
		object* ls = empty_list();
		while (!is_empty_list(values)) {
			
			ls = cons(evaluate_function_call(environment, f, cons(values->data.list.first, empty_list())), ls);
			values = list_rest(values);
		}
		return list_reverse(ls);
	}
}

object* evaluate_fold(object* environment, object* exp) {
	object* f = eval(environment, list_ref(1, exp));
	object* initial = eval(environment, list_ref(2, exp));
	object* values = eval(environment, list_ref(3, exp));
	while (!is_empty_list(values)) {
		initial = evaluate_function_call(environment, f, cons(values->data.list.first, cons(initial, empty_list())));
		values = list_rest(values);
	}
	return initial;
}

object* evaluate_filter(object* environment, object* exp) {
	object* f = eval(environment, list_ref(1, exp));
	object* values = eval(environment, list_ref(2, exp));
	object* ls = empty_list();
	while (!is_empty_list(values)) {
		object* first = values->data.list.first;
		object* b = evaluate_function_call(environment, f, cons(first, empty_list()));
		values = list_rest(values);
		if (!is_false(b)) {
			ls = cons(first, ls);
		}
	}
	return list_reverse(ls);
}

object* eval(object* environment, object* exp) {
	if (is_nonempty_list(exp)) {
		object* symbol = list_first(exp);
		if (is_define_symbol(symbol)) {
			return evaluate_definition(environment, exp);
		}
		else if (is_quote_symbol(symbol)) {
			return exp;
		}
		else if (is_lambda_symbol(symbol)) {
			return evaluate_lambda(environment, exp);
		}
		else if (is_map_symbol(symbol)) {
			return evaluate_map(environment, exp);
		}
		else if (is_fold_symbol(symbol)) {
			return evaluate_fold(environment, exp);
		}
		else if (is_filter_symbol(symbol)) {
			return evaluate_filter(environment, exp);
		}
		else if (is_curry_symbol(symbol)) {
			object* function_name = list_ref(1, exp);
			object* function = eval(environment, function_name);
			if (!is_function(function)) {
				printf("error: apply on non-function\n");
			}
			else {
				object* parameters = function_parameters(function);
				object* rev_param = list_reverse(parameters);
				if (is_empty_list(rev_param)) {
					return function;
				}
				else {
					object* body = function_body(function);
					while (!is_empty_list(rev_param)) {
						object* argument = cons(list_first(rev_param), empty_list());
						body = make_lambda(argument, body);
						rev_param = list_rest(rev_param);
					}
					return eval(environment, body);
				}
			}
		}
		else if (is_apply_symbol(symbol)) {
			object* function_name = list_ref(1, exp);
			object* function = eval(environment, function_name);
			if (!is_function(function)) {
				printf("error: apply on non-function\n");
			}
			else {
				int argument_count = list_length(exp) - 2;
				object* parameters = function_parameters(function);
				object* arguments = list_drop(2, exp);
				object* inner_parameters = list_drop(argument_count, parameters);
				object* outer_parameters = list_take(argument_count, parameters);
				object* inner_body = make_lambda(inner_parameters, function_body(function));
				object* outer_body = cons(make_lambda(outer_parameters, inner_body),  arguments);
				return eval(environment, outer_body);
			}
		}
		else if (is_if_symbol(symbol)) {
			object* condition = eval(environment, list_ref(1, exp));
			int pos;
			if (is_false(condition)) {
				pos = 3;
			}
			else {
				pos = 2;
			}
			return eval(environment, list_ref(pos, exp));
		}
		else if (is_list_symbol(symbol)) {
			return evaluate_values(environment, list_rest(exp));
		}
		else {
			object* first = eval(environment, symbol);
			if (is_no_object(first)) {
				printf("error: undefined binding\n");
			}
			else {
				if (is_function(first)) {
					object* values = evaluate_values(environment, list_rest(exp));
					return evaluate_function_call(environment, first, values);
				}
				else if (is_primitive_procedure(first)) {
					object* values = evaluate_values(environment, list_rest(exp));
					return (*(first->data.primitive_procedure.proc))(values);
				}
				else {
					printf("error: not a function\n");
				}
			}
		}
	}
	else if (is_symbol(exp)) {
		return binding_value(find_in_environment(environment, exp));
	}
	else {
		return exp;
	}
	return no_object();
}

void write(object* obj);

void write_list_cell(char first, object* obj) {
	if (is_empty_list(obj)) {
		printf("%c", list_end_delimiter[list_type(obj)]);
	}
	else {
		if (!first) {
			printf(" ");
		}
		write(list_first(obj));
		write_list_cell(0, list_rest(obj));
	}
}

void write_list(object* obj) {
	printf("%c", list_start_delimiter[list_type(obj)]);
	write_list_cell(1, obj);
}

void write(object* obj) {
	switch(obj->type) {
		case type_none:
			printf("undefined");
			break;
		case type_symbol:
			printf(symbol_name(obj));
			break;
		case type_boolean:
			if (is_false(obj)) {
				printf("#f");
			}
			else if (is_true(obj)) {
				printf("#t");
			}
			else {
				fprintf(stderr, "erroneous boolean");
			}
			break;
		case type_number:
			printf("%ld", number_value(obj));
			break;
		case type_list:
			if (is_quote_symbol(list_first(obj))) {
				printf("'");
				write(list_ref(1, obj));
			}
			else {
				write_list(obj);
			}
			break;
		case type_function:
			printf("function ");
			write_list(function_parameters(obj));
			break;
		default:
			fprintf(stderr, "unknown type");
	}
}

int main(void) {

	printf("running lisp interpreter. use ctrl-c to exit\n");
	
	init_global_variables();
	
	object* environment = standard_environment();
	object* ev;
	
	while(1) {
		printf("> ");
		ev = eval(environment, read(stdin));
		if (is_environment(ev)) {
			environment = ev;
		}
		else {
			write(ev);
			printf("\n");
		}
	}

	return 0;
}