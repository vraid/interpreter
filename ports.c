#include "ports.h"

#include <stdlib.h>
#include "data-structures.h"

int file_port_get_input(object* obj) {
	int c = getc(file_port_file(obj));
	obj->data.file_port.position += 1;
	obj->data.file_port.last_read_char = c;
	return c;
}

void file_port_unget_input(object* obj) {
	obj->data.file_port.position -= 1;
	ungetc(obj->data.file_port.last_read_char, file_port_file(obj));
}

int string_port_get_input(object* obj) {
	int n = string_port_position(obj);
	object* str = string_port_string(obj);
	obj->data.string_port.position += 1;
	char port_end = (n >= string_length(str));
	return port_end ? EOF : string_value(str)[n];
}

void string_port_unget_input(object* obj) {
	check_type(type_string_port, obj);
	obj->data.string_port.position -= 1;
}

int get_input(object* obj) {
	switch (obj->type) {
		case type_file_port:
			return file_port_get_input(obj);
			break;
		case type_string_port:
			return string_port_get_input(obj);
			break;
		default:
			printf("error: not a port (%s)\n", object_type_name(obj));
			return EOF;
	}
}

void unget_input(object* obj) {
	switch (obj->type) {
		case type_file_port:
			file_port_unget_input(obj);
			break;
		case type_string_port:
			string_port_unget_input(obj);
			break;
		default:
			printf("error: not a port (%s)\n", object_type_name(obj));
	}
}

int peek(object* obj) {
	int c = get_input(obj);
	unget_input(obj);
	return c;
}

long port_position(object* obj) {
	switch (obj->type) {
		case type_file_port:
			return file_port_position(obj);
		case type_string_port:
			return string_port_position(obj);
		default:
			printf("error: not a port (%s)\n", object_type_name(obj));
			return 0;
	}
}
