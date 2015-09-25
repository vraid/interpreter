#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {
	type_none,
	type_boolean,
	type_symbol,
	type_number,
	type_list,
	type_function,
	type_binding,
	type_environment} object_type;

typedef enum {round, square, curly, bracket_type_count} bracket_type;

typedef struct object {
	object_type type;
	union {
		struct {
			char value;
		} boolean;
		struct {
			char* name;
		} symbol;
		struct {
			long value;
		} number;
		struct {
			bracket_type type;
			struct object* first;
			struct object* rest;
		} list;
		struct {
			struct object* parameters;
			struct object* environment;
			struct object* body;
		} function;
		struct {
			struct object* name;
			struct object* value;
		} binding;
		struct {
			struct object* bindings;
		} environment;
	} data;
} object;

typedef enum {
	quote_symbol,
	define_symbol,
	lambda_symbol,
	curry_symbol,
	apply_symbol,
	if_symbol,
	list_symbol,
	special_symbol_count} special_symbol;

object* false;
object* true;
object* no_object;
object* no_symbol;
object* no_binding;
object* empty_list;
object* empty_lists[bracket_type_count];
object** special_symbols;
object* empty_environment;
char list_start_delimiter[bracket_type_count] = {'(', '[', '{'};
char list_end_delimiter[bracket_type_count] = {')', ']', '}'};

object** symbols;
int symbols_length;
int symbol_count;

object* allocate_object(void) {
	object* obj;
	
	obj = malloc(sizeof(object));
	if (obj == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	return obj;
}

object* allocate_object_type(object_type type) {
	object* obj = allocate_object();
	obj->type = type;
	return obj;
}

object* allocate_list_type(bracket_type type) {
	object* obj = allocate_object_type(type_list);
	obj->data.list.type = type;
	return obj;
}

object* allocate_list(void) {
	return allocate_list_type(round);
}

void check_type(object_type type, object* obj) {
	if (obj->type != type) {
		fprintf(stderr, "faulty type\n");
		exit(1);
	}
	return;
}

char is_boolean(object* obj) {
	return obj->type == type_boolean;
}

char boolean_value(object* obj) {
	check_type(type_boolean, obj);
	return obj->data.boolean.value;
}

char is_symbol(object* obj) {
	return obj->type == type_symbol;
}

char* symbol_name(object* obj) {
	check_type(type_symbol, obj);
	return obj->data.symbol.name;
}

char is_number(object* obj) {
	return obj->type == type_number;
}

long number_value(object* obj) {
	check_type(type_number, obj);
	return obj->data.number.value;
}

char is_list(object* obj) {
	return obj->type == type_list;
}

bracket_type list_type(object* obj) {
	check_type(type_list, obj);
	return obj->data.list.type;
}

object* list_first(object* ls) {
	check_type(type_list, ls);
	return ls->data.list.first;
}

object* list_rest(object* ls) {
	check_type(type_list, ls);
	return ls->data.list.rest;
}

char is_empty_list(object* obj) {
	return obj == empty_lists[list_type(obj)];
}

char is_nonempty_list(object* obj) {
	return is_list(obj) && !is_empty_list(obj);
}

object* cons(object* first, object* rest) {
	object* obj = allocate_list_type(list_type(rest));
	obj->data.list.first = first;
	obj->data.list.rest = rest;
	return obj;
}

object* list_ref(int n, object* ls) {
	while (n > 0) {
		ls = list_rest(ls);
		n--;
	}
	return list_first(ls);
}

object* list_take(int n, object* obj) {
	if (n == 0) {
		return empty_list;
	}
	else {
		object* ls = allocate_list();
		object* prev;
		object* next = ls;
		while (n > 0) {
			prev = next;
			prev->data.list.first = list_first(obj);
			obj = list_rest(obj);
			n--;
			if (n == 0) {
				next = empty_list;
			}
			else {
				next = allocate_list();
			}
			prev->data.list.rest = next;
		}
		return ls;
	}
}

object* list_drop(int n, object* obj) {
	while (n > 0) {
		obj = list_rest(obj);
		n--;
	}
	return obj;
}

object* append(object* as, object* rest) {
	if (is_empty_list(as)) {
		return rest;
	}
	else {
		object* ls = allocate_list();
		object* prev;
		object* next = ls;
		while (!is_empty_list(as)) {
			prev = next;
			prev->data.list.first = list_first(as);
			as = list_rest(as);
			if (is_empty_list(as)) {
				next = rest;
			}
			else {
				next = allocate_list();
			}
			prev->data.list.rest = next;
		}
		return ls;
	}
}

object* reverse(object* obj) {
	object* ls = empty_list;
	while (!is_empty_list(obj)) {
		ls = cons(list_first(obj), ls);
		obj = list_rest(obj);
	}
	return ls;
}

char is_function(object* obj) {
	return obj->type == type_function;
}

object* function_parameters(object* obj) {
	check_type(type_function, obj);
	return obj->data.function.parameters;
}

object* function_environment(object* obj) {
	check_type(type_function, obj);
	return obj->data.function.environment;
}

object* function_body(object* obj) {
	check_type(type_function, obj);
	return obj->data.function.body;
}

char is_binding(object* obj) {
	return obj->type == type_binding;
}

object* binding_name(object* obj) {
	check_type(type_binding, obj);
	return obj->data.binding.name;
}

object* binding_value(object* obj) {
	check_type(type_binding, obj);
	return obj->data.binding.value;
}

char is_environment(object* obj) {
	return obj->type == type_environment;
}

object* environment_bindings(object* obj) {
	check_type(type_environment, obj);
	return obj->data.environment.bindings;
}

char is_false(object* obj) {
	return obj == false;
}

char is_true(object* obj) {
	return obj == true;
}

char is_no_object(object* obj) {
	return obj == no_object;
}

char is_special_symbol(special_symbol s, object* obj) {
	return obj == special_symbols[s];
}

char is_either_special_symbol(object* obj) {
	int i;
	for (i = 0; i < special_symbol_count; i++) {
		if (is_special_symbol(i, obj)) {
			return 1;
		}
	}
	return 0;
}

char is_quote(object* obj) {
	return is_special_symbol(quote_symbol, obj);
}

char is_define(object* obj) {
	return is_special_symbol(define_symbol, obj);
}

char is_lambda(object* obj) {
	return is_special_symbol(lambda_symbol, obj);
}

char is_curry(object* obj) {
	return is_special_symbol(curry_symbol, obj);
}

char is_apply(object* obj) {
	return is_special_symbol(apply_symbol, obj);
}

char is_if(object* obj) {
	return is_special_symbol(if_symbol, obj);
}

char is_list_symbol(object* obj) {
	return is_special_symbol(list_symbol, obj);
}

object* allocate_object_boolean(char value) {
	object* obj;
	
	obj = allocate_object_type(type_boolean);
	obj->data.boolean.value = value;
	return obj;
}

object* make_symbol(char* name) {
	object* obj = allocate_object_type(type_symbol);
	obj->data.symbol.name = name;
	return obj;
}

object* add_symbol(char* name) {
	object* obj = make_symbol(name);
	if (symbols_length == symbol_count) {
		symbols_length = symbols_length * 2;
		symbols = realloc(symbols, symbols_length * sizeof(object*));
	}
	symbols[symbol_count] = obj;
	symbol_count++;
	return obj;
}

object* find_symbol(char* name) {
	int i;
	for (i = 0; i < symbol_count; i++) {
		if (strcmp(name, symbol_name(symbols[i])) == 0) {
			return symbols[i];
		}
	}
	return no_object;
}

object* make_empty_list(bracket_type type) {
	object* obj = allocate_list_type(type);
	obj->data.list.first = no_object;
	obj->data.list.rest = obj;
	return obj;
}

object* make_binding(object* name, object* value) {
	object* obj = allocate_object_type(type_binding);
	obj->data.binding.name = name;
	obj->data.binding.value = value;
	return obj;
}

object* make_environment(object* bindings) {
	object* obj = allocate_object_type(type_environment);
	obj->data.environment.bindings = bindings;
	return obj;
}

void init(void) {
	false = allocate_object_boolean(0);
	
	true = allocate_object_boolean(1);
	
	no_object = allocate_object_type(type_none);
	no_symbol = allocate_object_type(type_symbol);
	no_symbol->data.symbol.name = malloc(sizeof(char));
	no_symbol->data.symbol.name[0] = 0;
	
	no_binding = make_binding(no_symbol, no_object);
	
	int i;
	for (i = 0; i <= bracket_type_count; i++) {
		empty_lists[i] = make_empty_list(i);
	}
	
	empty_list = empty_lists[round];
	
	empty_environment = make_environment(empty_list);
	
	symbols_length = 1;
	symbol_count = 0;
	symbols = malloc(sizeof(object*));
	
	special_symbols = malloc(special_symbol_count * sizeof(object*));
	
	special_symbols[quote_symbol] = add_symbol("quote");
	special_symbols[define_symbol] = add_symbol("define");
	special_symbols[lambda_symbol] = add_symbol("lambda");
	special_symbols[curry_symbol] = add_symbol("curry");
	special_symbols[apply_symbol] = add_symbol("apply");
	special_symbols[if_symbol] = add_symbol("if");
	special_symbols[list_symbol] = add_symbol("list");
}

int list_length(object* ls) {
	int n = 0;
	while (!is_empty_list(ls)) {
		n++;
		ls = list_rest(ls);
	}
	return n;
}

object* make_binding_list(object* names, object* values) {
	if (is_empty_list(names)) {
		return empty_list;
	}
	else {
		object* ls = allocate_list();
		object* prev;
		object* next = ls;
		while (!is_empty_list(names)) {
			prev = next;
			prev->data.list.first = make_binding(list_first(names), list_first(values));
		
			names = list_rest(names);
			values = list_rest(values);
		
			if (is_empty_list(names)) {
				next = empty_list;
			}
			else {
				next = allocate_list();
			}
			prev->data.list.rest = next;
		}
		return ls;
	}
}

object* extend_environment(object* env, object* bindings) {
	if (is_empty_list(bindings)) {
		return env;
	}
	else {
		return make_environment(append(bindings, environment_bindings(env)));
	}
}

object* find_in_environment(object* env, object* symbol) {
	object* ls = environment_bindings(env);
	while (!is_empty_list(ls)) {
		object* binding = list_first(ls);
		if (symbol == binding_name(binding)) {
			return binding;
		}
		ls = list_rest(ls);
	}
	return no_binding;
}

char is_self_quoting(object* obj) {
	switch(obj->type) {
		case type_boolean:
		case type_number:
			return 1;
		default:
			return 0;
	}
}

object* quote(object* value) {
	return cons(special_symbols[quote_symbol], cons(value, empty_list));
}

char list_starts_with(object* ls, object* obj) {
	return is_nonempty_list(ls) && (obj == list_first(ls));
}

char is_quoted(object* exp) {
	return list_starts_with(exp, special_symbols[quote_symbol]);
}

char is_definition(object* exp) {
	return list_starts_with(exp, special_symbols[define_symbol]);
}

object* make_number(long value) {
	object* obj = allocate_object_type(type_number);
	obj->data.number.value = value;
	return obj;
}

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
	if (obj == no_object) {
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
				return true;
			case 'f':
				return false;
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
		return empty_lists[type];
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
		return empty_list;
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
				next = empty_list;
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
	return extend_environment(environment, cons(binding, empty_list));
}

object* make_lambda(object* parameters, object* body) {
	return cons(special_symbols[lambda_symbol], cons(parameters, cons(body, empty_list)));
}

object* evaluate_lambda(object* environment, object* exp) {
	object* parameters = list_ref(1, exp);
	object* body = list_ref(2, exp);
	object* obj = allocate_object_type(type_function);
	obj->data.function.parameters = parameters;
	obj->data.function.environment = environment;
	obj->data.function.body = body;
	return obj;
}

object* eval(object* environment, object* exp) {
	if (is_nonempty_list(exp)) {
		object* symbol = list_first(exp);
		if (is_define(symbol)) {
			return evaluate_definition(environment, exp);
		}
		else if (is_quote(symbol)) {
			return exp;
		}
		else if (is_lambda(symbol)) {
			return evaluate_lambda(environment, exp);
		}
		else if (is_curry(symbol)) {
			object* function_name = list_ref(1, exp);
			object* function = eval(environment, function_name);
			if (!is_function(function)) {
				printf("error: apply on non-function\n");
			}
			else {
				object* parameters = function_parameters(function);
				object* rev_param = reverse(parameters);
				if (is_empty_list(rev_param)) {
					return function;
				}
				else {
					object* body = function_body(function);
					while (!is_empty_list(rev_param)) {
						object* argument = cons(list_first(rev_param), empty_list);
						body = make_lambda(argument, body);
						rev_param = list_rest(rev_param);
					}
					return eval(environment, body);
				}
			}
		}
		else if (is_apply(symbol)) {
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
		else if (is_if(symbol)) {
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
			return quote(evaluate_values(environment, list_rest(exp)));
		}
		else {
			object* first = eval(environment, symbol);
			if (is_no_object(first)) {
				printf("error: undefined binding\n");
			}
			else {
				if (!is_function(first)) {
					printf("error: not a function\n");
				}
				else {
					object* values = evaluate_values(environment, list_rest(exp));
					object* bindings = make_binding_list(function_parameters(first), values);
					return eval(extend_environment(function_environment(first), bindings), function_body(first));
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
	return no_object;
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
			if (is_quoted(obj)) {
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
	
	init();
	
	object* environment = empty_environment;
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