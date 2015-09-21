#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {
	type_quote,
	type_boolean,
	type_symbol,
	type_number,
	type_list_cell} object_type;


typedef struct object {
	object_type type;
	union {
		struct {
			struct object* value;
		} quote;
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
			struct object* first;
			struct object* rest;
		} list_cell;
	} data;
} object;

object* false;
object* true;
object* empty_list;

char is_boolean(object* obj) {
	return obj->type == type_boolean;
}

char is_false(object* obj) {
	return obj == false;
}

char is_true(object* obj) {
	return obj == true;
}

char is_empty_list(object* obj) {
	return obj == empty_list;
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

object* allocate_object_boolean(char value) {
	object* obj;
	
	obj = allocate_object_type(type_boolean);
	obj->data.boolean.value = value;
	return obj;
}

object** symbols;
int symbols_length;
int symbol_count;

void init(void) {
	false = allocate_object_boolean(0);
	
	true = allocate_object_boolean(1);
	
	empty_list = allocate_object_type(type_list_cell);
	
	symbols_length = 1;
	symbol_count = 0;
	symbols = malloc(sizeof(object*));
}

char is_self_quoting(object* obj) {
	switch(obj->type) {
		case type_quote:
		case type_boolean:
		case type_number:
			return 1;
		default:
			return 0;
	}
}

object* make_quote(object* value) {
	object* obj = allocate_object_type(type_quote);
	obj->data.quote.value = value;
	return obj;
}

object* quote(object* obj) {
	if (is_self_quoting(obj)) {
		return obj;
	}
	else {
		return make_quote(obj);
	}
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
	return NULL;
}

object* make_number(long value) {
	object* obj = allocate_object_type(type_number);
	obj->data.number.value = value;
	return obj;
}

object* cons(object* first, object* rest) {
	object* obj = allocate_object_type(type_list_cell);
	obj->data.list_cell.first = first;
	obj->data.list_cell.rest = rest;
	return obj;
}

char is_quote(object* obj) {
	return obj->type == type_quote;
}

char is_number(object* obj) {
	return obj->type == type_number;
}

char is_list_cell(object* obj) {
	return obj->type == type_list_cell;
}

char is_list(object* obj) {
	return is_list_cell(obj) || is_empty_list(obj);
}

char is_delimiter(int c) {
	return isspace(c) || c == EOF ||
		c == '(' || c == ')' ||
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
	char quoted;
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

object* read_list(FILE* in, read_state state);

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
	if (obj == NULL) {
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
		if (state.quoted) {
			return read_symbol(in);
		}
		else {
			fprintf(stderr, "unquoted identifiers not supported\n");
			exit(1);
		}
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
		if (!state.quoted) {
			state.quoted = 1;
			return quote(read_value(in, state));
		}
		else {
			return read_value(in, state);
		}
	}
	else if (c == '(') {
		getc(in);
		return read_list(in, state);
	}
	else {
		return read_atom(in, state);
	}
	fprintf(stderr, "read illegal state\n");
	exit(1);
}

object* read_list(FILE* in, read_state state) {
	int c;
	
	trim_whitespace(in);
	
	c = peek(in);
	
	if (c == ')') {
		getc(in);
		return empty_list;
	}
	else {
		object* first = read_value(in, state);
		return cons(first, read_list(in, state));
	}
}

object* read(FILE* in) {
	read_state state;
	state.quoted = 0;
	
	return read_value(in, state);
}

object* eval(object* exp) {
	return exp;
}

void write(object* obj);

void write_list_cell(char first, object* obj) {
	if (is_empty_list(obj)) {
		printf(")");
	}
	else {
		if (!first) {
			printf(" ");
		}
		write(obj->data.list_cell.first);
		write_list_cell(0, obj->data.list_cell.rest);
	}
}

void write_list(object* obj) {
	printf("(");
	write_list_cell(1, obj);
}

void write(object* obj) {
	switch(obj->type) {
		case type_quote:
			printf("'");
			write(obj->data.quote.value);
			break;
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
		case type_list_cell:
			write_list(obj);
			break;
		default:
			fprintf(stderr, "unknown type\n");
			exit(1);
	}
}

int main(void) {

	printf("running lisp interpreter. use ctrl-c to exit");
	
	init();
	
	while(1) {
		printf("> ");
		write(eval(read(stdin)));
		printf("\n");
	}

	return 0;
}