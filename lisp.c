#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "data-structures.c"
#include "allocation.c"
#include "standard-library.c"
#include "global-variables.c"
#include "environments.c"
#include "list-util.c"

char in_bracket_list(char* ls, int c) {
	int i;
	for (i = 0; i < bracket_type_count; i++) {
		if (c == ls[i]) {
			return 1;
		}
	}
	return 0;
}

char is_list_start_delimiter(int c) {
	return in_bracket_list(list_start_delimiter, c);
}

char is_list_end_delimiter(int c) {
	return in_bracket_list(list_end_delimiter, c);
}

char is_list_delimiter(int c) {
	return is_list_start_delimiter(c) || is_list_end_delimiter(c);
}

bracket_type list_delimiter_type(int c) {
	switch (c) {
		case '(':
		case ')': return round;
		case '[':
		case ']': return square;
		case '{':
		case '}': return curly;
		default: return bracket_type_count;
	}
}

char is_delimiter(int c) {
	return isspace(c) || c == EOF ||
		is_list_delimiter(c) ||
		c == '"' || c == ';';
}

int peek(FILE* in) {
	int c = getc(in);
	ungetc(c, in);
	return c;
}

void trim_whitespace(FILE* in) {
	int c;
	
	while ((c = getc(in)) != EOF) {
		if (isspace(c)) {
			continue;
		}
		else if (c == ';') {
			while (((c = getc(in)) != EOF) && (c != '\n')) {}
			continue;
		}
		ungetc(c, in);
		break;
	}
}

char* duplicate_string(char* str) {
	size_t length = strlen(str) + 1;
	char* d = malloc(length);
	if (d == NULL) {
		fprintf(stderr, "out of memory\n");
		return NULL;
	}
	memcpy(d, str, length);
	return d;
}

typedef struct read_state {
} read_state;

char* read_identifier(FILE* in) {
	char buffer[1000];
	int i = 0;
	int c;
	while (!is_delimiter(c = peek(in))) {
		buffer[i] = c;
		getc(in);
		i++;
	}
	buffer[i] = '\0';
	return duplicate_string(buffer);
}

object* read_list(FILE* in, read_state state, bracket_type type);

object* read_number(FILE* in) {
	int c;
	long num = 0;
	while (isdigit(c = getc(in))) {
		num = (num * 10) + (c - '0');
	}
	if (is_delimiter(c)) {
		ungetc(c, in);
		return make_number(num);
	}
	else {
		fprintf(stderr, "number not followed by delimiter\n");
		exit(1);
	}
}

object* read_symbol(FILE* in) {
	char* name = read_identifier(in);
	object* obj = find_symbol(name);
	if (is_no_object(obj)) {
		obj = add_symbol(name);
	}
	else {
		free(name);
	}
	return obj;
}

object* read_atom(FILE* in, read_state state) {
	int c;
	
	c = peek(in);
	if (c == '#') {
		getc(in);
		c = getc(in);
		switch(c) {
			case 't':
				return true();
			case 'f':
				return false();
			default:
				fprintf(stderr, "unknown type\n");
				exit(1);
		}
	}
	else if (isdigit(c)) {
		return read_number(in);
	}
	else if (!is_delimiter(c)) {
		return read_symbol(in);
	}
	
	fprintf(stderr, "read illegal state\n");
	exit(1);
}

object* read_value(FILE* in, read_state state) {
	int c;

	trim_whitespace(in);
	
	c = peek(in);
	
	if (c == '\'') {
		getc(in);
		c = peek(in);
		return quote(read_value(in, state));
	}
	else if (is_list_start_delimiter(c)) {
		getc(in);
		return read_list(in, state, list_delimiter_type(c));
	}
	else {
		return read_atom(in, state);
	}
	fprintf(stderr, "read illegal state\n");
	exit(1);
}

object* read_list(FILE* in, read_state state, bracket_type type) {
	int c;
	
	trim_whitespace(in);
	
	c = peek(in);
	
	if (is_list_end_delimiter(c)) {
		getc(in);
		if (!(type == list_delimiter_type(c))) {
			fprintf(stderr, "bracket mismatch\n");
			exit(1);
		}
		return empty_list_type(type);
	}
	else {
		object* first = read_value(in, state);
		return cons(first, read_list(in, state, type));
	}
}

object* read(FILE* in) {
	read_state state;
	
	return read_value(in, state);
}

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
		object* ls = allocate_list();
		object* prev;
		object* next = ls;
		while (!is_empty_list(values)) {
			prev = next;
			prev->data.list.first = evaluate_function_call(environment, f, cons(values->data.list.first, empty_list()));
			values = list_rest(values);
			if (is_empty_list(values)) {
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
	object* next;
	while (!is_empty_list(values)) {
		object* first = values->data.list.first;
		object* b = evaluate_function_call(environment, f, cons(first, empty_list()));
		values = list_rest(values);
		if (!is_false(b)) {
			if (is_empty_list(ls)) {
				ls = allocate_list();
				next = ls;
			}
			else {
				next->data.list.rest = allocate_list();
				next = list_rest(next);
			}
			next->data.list.first = first;
		}
		next->data.list.rest = empty_list();
	}
	return ls;
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