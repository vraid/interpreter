#include <stdlib.h>
#include <string.h>
#include "symbols.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"

#define static_symbol_max 1024

object static_symbols[static_symbol_max];
object static_symbol_cells[static_symbol_max];
int static_symbol_count;

object* make_static_symbol(char* name) {
	object* string = malloc(sizeof(object));
	init_string(string, name);
	string->location = location_static;
	object* obj = &static_symbols[static_symbol_count];
	init_symbol(obj, string);
	obj->location = location_static;
	object* ls = &static_symbol_cells[static_symbol_count];
	init_list_cell(ls, obj, symbol_list);
	ls->location = location_static;
	symbol_list = ls;
	static_symbol_count++;
	return obj;
}

object* symbol(char* name, object* cont) {
	// look for existing symbol first
	object* ls = symbol_list;
	while (ls != empty_list()) {
		object* symbol = list_first(ls);
		if (strcmp(name, string_value(symbol_name(symbol))) == 0) {
			return call_cont(cont, symbol);
		}
		ls = list_rest(ls);
	}
	// symbol not found, adding it
	object string;
	char* str = alloca(sizeof(char) * (1 + strlen(name)));
	strcpy(str, name);
	init_string(&string, str);
	object obj;
	init_symbol(&obj, &string);
	object cell;
	init_list_cell(&cell, &obj, symbol_list);
	symbol_list = &cell;
	
	return call_cont(cont, &obj);
}

void init_symbols(void) {
	symbol_list = empty_list();
	static_symbol_count = 0;
}
