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

void reader_trim_whitespace(reader_object* reader) {
	FILE* in = (*reader).in;
	int c;
	
	while ((c = getc(in)) != EOF) {
		(*reader).state.char_index++;
		if (isspace(c)) {
			continue;
		}
		else if (c == ';') {
			while (((c = getc(in)) != EOF) && (c != '\n')) {}
			continue;
		}
		ungetc(c, in);
		(*reader).state.char_index--;
		break;
	}
}

char reader_peek(reader_object* reader) {
	return peek((*reader).in);
}

char reader_get(reader_object* reader) {
	(*reader).state.char_index++;
	return getc((*reader).in);
}

char* read_identifier(reader_object* reader) {
	char buffer[1000];
	int i = 0;
	int c;
	while (!is_delimiter(c = reader_peek(reader))) {
		buffer[i] = c;
		reader_get(reader);
		i++;
	}
	buffer[i] = '\0';
	return duplicate_string(buffer);
}

object* read_list(reader_object* reader, bracket_type type);

object* read_number(reader_object* reader) {
	int c;
	long num = 0;
	while (isdigit(c = reader_peek(reader))) {
		reader_get(reader);
		num = (num * 10) + (c - '0');
	}
	if (is_delimiter(c)) {
		return make_number(num);
	}
	else {
		fprintf(stderr, "number not followed by delimiter\n");
		exit(1);
	}
}

object* read_symbol(reader_object* reader) {
	char* name = read_identifier(reader);
	object* obj = find_symbol(name);
	if (is_no_object(obj)) {
		obj = add_symbol(name);
	}
	else {
		free(name);
	}
	return obj;
}

object* read_atom(reader_object* reader) {
	int c;
	
	c = reader_peek(reader);
	if (c == '#') {
		reader_get(reader);
		c = reader_get(reader);
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
		return read_number(reader);
	}
	else if (!is_delimiter(c)) {
		return read_symbol(reader);
	}
	
	fprintf(stderr, "read illegal state\n");
	exit(1);
}

object* read_value(reader_object* reader) {
	int c;

	reader_trim_whitespace(reader);
	c = reader_peek(reader);
	
	if (c == '\'') {
		reader_get(reader);
		c = reader_peek(reader);
		return quote(read_value(reader));
	}
	else if (is_list_start_delimiter(c)) {
		reader_get(reader);
		return read_list(reader, list_delimiter_type(c));
	}
	else {
		return read_atom(reader);
	}
	fprintf(stderr, "read illegal state\n");
	exit(1);
}

object* read_list(reader_object* reader, bracket_type type) {
	int c;
	
	c = reader_peek(reader);
	
	if (is_list_end_delimiter(c)) {
		reader_get(reader);
		if (!(type == list_delimiter_type(c))) {
			fprintf(stderr, "bracket mismatch\n");
			exit(1);
		}
		return empty_list_type(type);
	}
	else {
		object* first = read_value(reader);
		return cons(first, read_list(reader, type));
	}
}

object* read(FILE* in) {
	reader_object reader;
	reader.in = in;
	
	return read_value(&reader);
}
