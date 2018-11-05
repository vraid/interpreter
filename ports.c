#include "ports.h"

#include "data-structures.h"

char get_input(object* obj) {
	char c = getc(file_port_file(obj));
	obj->data.file_port.position += 1;
	obj->data.file_port.last_read_char = c;
	return c;
}

void unget_input(object* obj) {
	obj->data.file_port.position -= 1;
	ungetc(obj->data.file_port.last_read_char, file_port_file(obj));
}

char peek(object* obj) {
	char c = get_input(obj);
	unget_input(obj);
	return c;
}
