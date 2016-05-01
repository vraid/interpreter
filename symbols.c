#include <stdlib.h>
#include <string.h>
#include "symbols.h"
#include "data-structures.h"
#include "allocation.h"

object** symbols;
int symbols_length;
int symbol_count;

void init_symbols(void) {
	symbols_length = 1024;
	symbol_count = 0;
	symbols = malloc(symbols_length * sizeof(object*));
}

void add_symbol(object* obj) {
	if (symbols_length == symbol_count) {
		symbols_length = symbols_length * 2;
		symbols = realloc(symbols, symbols_length * sizeof(object*));
	}
	symbols[symbol_count] = obj;
	symbol_count++;
}

void init_symbol(object* obj, char* name) {
	init_object(location_static, type_symbol, obj);
	obj->data.symbol.name = name;
}

void make_static_symbol(object* obj, char* name) {
	init_symbol(obj, name);
	add_symbol(obj);
}

object* symbol(char* name) {
	// look for existing symbol first
	for (int i = 0; i < symbol_count; i++) {
		if (strcmp(name, symbol_name(symbols[i])) == 0) {
			return symbols[i];
		}
	}
	// symbol not found, adding it
	object* obj = malloc(sizeof(object));
	char* str = malloc(sizeof(char) * strlen(name));
	strcpy(str, name);
	init_symbol(obj, str);
	add_symbol(obj);
	
	return obj;
}
