#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "read.h"
#include "global-variables.h"

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