
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {type_number} object_type;

typedef struct object {
	object_type type;
	union {
		struct {
			long value;
		} number;
	} data;
} object;

object* allocate_object(void) {
	object* obj;
	
	obj = malloc(sizeof(object));
	if (obj == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	return obj;
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

	while(1) {
		printf("> ");
		write(eval(read(stdin)));
		printf("\n");
	}

	return 0;
}