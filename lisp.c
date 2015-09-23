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
	type_variable,
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
			struct object* name;
			struct object* value;
		} variable;
		struct {
			int element_count;
			struct object** elements;
		} environment;
	} data;
} object;

object* false;
object* true;
object* no_object;
object* empty_list[bracket_type_count];
object* quote_symbol;
object* define_symbol;
char list_start_delimiter[bracket_type_count] = {'(', '[', '{'};
char list_end_delimiter[bracket_type_count] = {')', ']', '}'};

object** symbols;
int symbols_length;
int symbol_count;

char is_boolean(object* obj) {
	return obj->type == type_boolean;
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

char is_quote_symbol(object* obj) {
	return obj == quote_symbol;
}

char is_list(object* obj) {
	return obj->type == type_list;
}

char is_empty_list(object* obj) {
	return obj == empty_list[obj->data.list.type];
}

char is_nonempty_list(object* obj) {
	return is_list(obj) && !is_empty_list(obj);
}

char is_environment(object* obj) {
	return obj->type == type_environment;
}

char is_environment_list(object* obj) {
	return is_nonempty_list(obj) && is_environment(obj->data.list.first);
}

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
	object* obj;
	
	obj = allocate_object();
	obj->type = type;
	return obj;
}

object* cons(object* first, object* rest) {
	object* obj = allocate_object_type(type_list);
	obj->data.list.type = rest->data.list.type;
	obj->data.list.first = first;
	obj->data.list.rest = rest;
	return obj;
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
		if (strcmp(name, symbols[i]->data.symbol.name) == 0) {
			return symbols[i];
		}
	}
	return no_object;
}

object* make_empty_list(bracket_type type) {
	object* obj = allocate_object_type(type_list);
	obj->data.list.type = type;
	return obj;
}

void init(void) {
	false = allocate_object_boolean(0);
	
	true = allocate_object_boolean(1);
	
	no_object = allocate_object_type(type_none);
	
	int i;
	for (i = 0; i < bracket_type_count; i++) {
		empty_list[i] = make_empty_list(i);
	}
	
	symbols_length = 1;
	symbol_count = 0;
	symbols = malloc(sizeof(object*));
	
	quote_symbol = add_symbol("quote");
	define_symbol = add_symbol("define");
}

object* variable_name(object* obj) {
	return obj->data.variable.name;
}

object* variable_value(object* obj) {
	return obj->data.variable.value;
}

object* make_environment(object** elements, int element_count) {
	object** elem = malloc(element_count * sizeof(object*));
	memcpy(elem, elements, element_count * sizeof(object*));
	
	object* obj = allocate_object_type(type_environment);
	obj->data.environment.elements = elem;
	obj->data.environment.element_count = element_count;
	return obj;
}

object* find_in_environment(object* env, object* symbol) {
	int i;
	int count = env->data.environment.element_count;
	object** elem = env->data.environment.elements;
	for (i = 0; i < count; i++) {
		if (symbol == variable_name(elem[i])) {
			return elem[i];
		}
	}
	return no_object;
}

object* find_in_environment_list(object* ls, object* symbol) {
	object* obj = no_object;
	while (!is_empty_list(ls) && (obj == no_object)) {
		obj = find_in_environment(ls->data.list.first, symbol);
		if (obj == no_object) {
			ls = ls->data.list.rest;
		}
	}
	return obj;
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
	return cons(quote_symbol, cons(value, empty_list[round]));
}

char is_quoted(object* obj) {
	return is_nonempty_list(obj) && is_quote_symbol(obj->data.list.first);
}

object* make_number(long value) {
	object* obj = allocate_object_type(type_number);
	obj->data.number.value = value;
	return obj;
}

char is_number(object* obj) {
	return obj->type == type_number;
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
		fprintf(stderr, "out of memory");
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
		return empty_list[type];
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

object* list_ref(int n, object* obj) {
	if (n == 0) {
		return obj->data.list.first;
	}
	else {
		return list_ref(n - 1, obj->data.list.rest);
	}
}

object* eval(object* exp) {
	return exp;
}

void write(object* obj);

void write_list_cell(char first, object* obj) {
	if (is_empty_list(obj)) {
		printf("%c", list_end_delimiter[obj->data.list.type]);
	}
	else {
		if (!first) {
			printf(" ");
		}
		write(obj->data.list.first);
		write_list_cell(0, obj->data.list.rest);
	}
}

void write_list(object* obj) {
	printf("%c", list_start_delimiter[obj->data.list.type]);
	write_list_cell(1, obj);
}

void write(object* obj) {
	switch(obj->type) {
		case type_symbol:
			printf(obj->data.symbol.name);
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
			printf("%ld", obj->data.number.value);
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
		default:
			fprintf(stderr, "unknown type\n");
			exit(1);
	}
}

int main(void) {

	printf("running lisp interpreter. use ctrl-c to exit");
	
	init();
	
	object* ev;
	
	while(1) {
		printf("> ");
		ev = eval(read(stdin));
		if (is_environment_list(ev)) {
		}
		else {
			write(ev);
		}
		printf("\n");
	}

	return 0;
}