#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {
	type_boolean,
	type_symbol,
	type_number,
	type_list_cell} object_type;


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

void init(void) {
	false = allocate_object_boolean(0);
	
	true = allocate_object_boolean(1);
	
	empty_list = allocate_object_type(type_list_cell);
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

object* make_number(long value) {
	object* obj;
	
	obj = allocate_object_type(type_number);
	obj->data.number.value = value;
	return obj;
}

object* cons(object* first, object* rest) {
	object* obj;
	
	obj = allocate_object_type(type_list_cell);
	obj->data.list_cell.first = first;
	obj->data.list_cell.rest = rest;
	return obj;
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

typedef struct read_state {
	char quoted;
} read_state;

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

object* read_atom(FILE* in, read_state state) {
	int c;
	
	c = peek(in);
	
	if (isdigit(c)) {
		return read_number(in);
	}
	else if (c == '#') {
		getc(in);
		c = getc(in);
		switch(c) {
			case 't':
				return true;
			case 'f':
				return false;
			default:
				fprintf(stderr, "unknown type\n");
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
		state.quoted = 1;
		return read_value(in, state);
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