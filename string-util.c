#include "string-util.h"

#include <stdlib.h>
#include <string.h>
#include "data-structures.h"
#include "object-init.h"

char* copy_str(char* dest, char* str) {
	return strcpy(dest, str);
}

int str_length(char* str) {
	return strlen(str);
}

char* init_str(char* str) {
	str[0] = '\0';
	return str;
}

char* cat(char* str, int count, object** arr) {
	init_str(str);
	for (int i = 0; i < count; i++) {
		strcat(str, string_value(arr[i]));
	}
	return str;
}

object* append(object* obj, char* str, int count, object** arr) {
	cat(str, count, arr);
	return init_string(obj, str);
}

object* append_string_2(object* obj, char* str, object* a, object* b) {
	object* arr[2] = {a, b};
	return append(obj, str, 2, arr);
}

object* append_string_3(object* obj, char* str, object* a, object* b, object* c) {
	object* arr[3] = {a, b, c};
	return append(obj, str, 3, arr);
}
