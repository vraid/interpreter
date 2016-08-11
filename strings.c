#include "strings.h"

#include <stdlib.h>
#include <string.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"

object* string_append(object* args, object* cont) {
	object* strings;
	delist_1(args, &strings);
	
	object* ls = strings;
	int length = 0;
	while (!is_empty_list(ls)) {
		length += string_length(list_first(ls));
		ls = list_rest(ls);
	}
	char* str = alloca(sizeof(char) * (length + 1));
	int i = 0;
	ls = strings;
	while (!is_empty_list(ls)) {
		object* s = list_first(ls);
		int len = string_length(s);
		memcpy(str+i, string_value(s), len);
		i += len;
		ls = list_rest(ls);
	}
	str[i] = 0;
	
	object new_string;
	init_string(&new_string, str);
	
	return call_cont(cont, &new_string);
}

void init_string_procedures(void) {
	init_primitive(&string_append, &string_append_proc);
}
