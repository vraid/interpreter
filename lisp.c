
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {type_boolean, type_number} object_type;

typedef struct object {
	object_type type;
	union {
		struct {
			char value;
		} boolean;
		struct {
			long value;
		} number;
	} data;
} object;

object* false;
object* true;

char is_boolean(object* obj) {
	return obj->type == type_boolean;
}

char is_false(object* obj) {
	return obj == false;
}

char is_true(object* obj) {
	return obj == true;
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

void init(void) {
	false = allocate_object();
	false->type = type_boolean;
	false->data.boolean.value = 0;
	
	true = allocate_object();
	true->type = type_boolean;
	true->data.boolean.value = 1;
}

object* make_number(long value) {
	object* obj;
	
	obj = allocate_object();
	obj->type = type_number;
	obj->data.number.value = value;
	return obj;
}

char is_number(object* obj) {
	return obj->type == type_number;
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

object* read(FILE* in) {
	int c;
	long num = 0;

	trim_whitespace(in);
	
	c = getc(in);
	
	if (isdigit(c)) {
		while (isdigit(c)) {
			num = (num * 10) + (c - '0');
			c = getc(in);
		}
		if (is_delimiter(c)) {
			return make_number(num);
		}
		else {
			fprintf(stderr, "number not followed by delimiter\n");
			exit(1);
		}
	}
	else if (c == '#') {
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
	else {
		fprintf(stderr, "bad input, unexpected '%c'\n", c);
		exit(1);
	}
	fprintf(stderr, "read illegal state\n");
	exit(1);
}

object* eval(object* exp) {
	return exp;
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