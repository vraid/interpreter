#include <stdlib.h>
#include <string.h>
#include "symbols.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"
#include "string-util.h"

#define static_symbol_max 1024

object static_symbols[static_symbol_max];
object static_symbol_cells[static_symbol_max];
object static_symbol_names[static_symbol_max];
int static_symbol_count;

object* find_symbol(char* name) {
	object* ls = symbol_list;
	while (ls != empty_list()) {
		object* symbol = list_first(ls);
		if (strcmp(name, string_value(symbol_name(symbol))) == 0) {
			return symbol;
		}
		ls = list_rest(ls);
	}
	return no_object();
}

object* make_static_symbol(char* name) {
	object* obj = find_symbol(name);
	
	if (is_no_object(obj)) {
		object* string = &static_symbol_names[static_symbol_count];
		init_string(string, name);
		make_static(string);
		obj = &static_symbols[static_symbol_count];
		init_symbol(obj, string);
		make_static(obj);
		object* ls = &static_symbol_cells[static_symbol_count];
		init_list_cell(ls, obj, symbol_list);
		make_static(ls);
		symbol_list = ls;
		static_symbol_count++;
	}
	return obj;
}

object* symbol(char* name, object* cont) {
	// look for existing symbol first
	object* obj = find_symbol(name);
	
	if (is_no_object(obj)) {
		// symbol not found, adding it
		char* str = alloc_copy_str(name);
		object* string = alloc_string(str);
		object* symbol = alloc_symbol(string);
		object* cell = alloc_list_cell(symbol, symbol_list);
		symbol_list = cell;
		obj = symbol;
	}
	
	return call_cont(cont, obj);
}

object* string_to_symbol(object* args, object* cont) {
	object* str;
	delist_1(args, &str);
	
	return symbol(string_value(str), cont);
}

object* symbol_to_string(object* args, object* cont) {
	object* symbol;
	delist_1(args, &symbol);
	
	return call_cont(cont, symbol_name(symbol));
}

void init_symbols(void) {
	symbol_list = empty_list();
	static_symbol_count = 0;
	init_primitive(&string_to_symbol, &string_to_symbol_proc);
	init_primitive(&symbol_to_string, &symbol_to_string_proc);
}
