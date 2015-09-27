#include "global-variables.h"
#include "data-structures.h"

object** symbols;
int symbols_length;
int symbol_count;

object* _false;
object* _true;
object* _no_object;
object* _no_symbol;
object* _no_binding;
object* _empty_list;
object* empty_lists[bracket_type_count];
object* _quote_symbol;
object* _define_symbol;
object* _lambda_symbol;
object* _curry_symbol;
object* _apply_symbol;
object* _if_symbol;
object* _list_symbol;
object* _empty_environment;

void init_global_variables(void) {
	_false = allocate_object_boolean(0);
	
	_true = allocate_object_boolean(1);
	
	_no_object = allocate_object_type(type_none);
	_no_symbol = allocate_object_type(type_symbol);
	_no_symbol->data.symbol.name = malloc(sizeof(char));
	_no_symbol->data.symbol.name[0] = 0;
	
	_no_binding = make_binding(no_symbol(), no_object());
	
	int i;
	for (i = 0; i <= bracket_type_count; i++) {
		empty_lists[i] = make_empty_list(i);
	}
	
	_empty_list = empty_lists[shapeless];
	
	_empty_environment = make_environment(empty_list());
	
	symbols_length = 1;
	symbol_count = 0;
	symbols = malloc(sizeof(object*));
	
	_quote_symbol = add_symbol("quote");
	_define_symbol = add_symbol("define");
	_lambda_symbol = add_symbol("lambda");
	_curry_symbol = add_symbol("curry");
	_apply_symbol = add_symbol("apply");
	_if_symbol = add_symbol("if");
	_list_symbol = add_symbol("list");
}

object* find_symbol(char* name) {
	int i;
	for (i = 0; i < symbol_count; i++) {
		if (strcmp(name, symbol_name(symbols[i])) == 0) {
			return symbols[i];
		}
	}
	return no_object();
}

object* add_symbol(char* name) {
	object* obj = make_symbol(name);
	if (symbols_length == symbol_count) {
		symbols_length = symbols_length * 2;
		symbols = realloc(symbols, symbols_length * sizeof(object*));
	}
	symbols[symbol_count] = obj;
	symbol_count++;
	return obj;
}

object* true(void) {
	return _true;
}
object* false(void) {
	return _false;
}
char is_empty_list(object* obj) {
	return obj == empty_lists[list_type(obj)];
}
char is_false(object* obj) {
	return obj == _false;
}
char is_true(object* obj) {
	return obj == _true;
}
char is_no_object(object* obj) {
	return obj == _no_object;
}
object* empty_list(void) {
	return _empty_list;
}
object* empty_list_type(bracket_type type) {
	return empty_lists[type];
}
object* no_object(void) {
	return _no_object;
}
object* no_symbol(void) {
	return _no_symbol;
}
object* no_binding(void) {
	return _no_binding;
}
object* empty_environment(void) {
	return _empty_environment;
}

char is_quote_symbol(object* obj) {
	return obj == _quote_symbol;
}
char is_define_symbol(object* obj) {
	return obj == _define_symbol;
}
char is_lambda_symbol(object* obj) {
	return obj == _lambda_symbol;
}
char is_curry_symbol(object* obj) {
	return obj == _curry_symbol;
}
char is_apply_symbol(object* obj) {
	return obj == _apply_symbol;
}
char is_if_symbol(object* obj) {
	return obj == _if_symbol;
}
char is_list_symbol(object* obj) {
	return obj == _list_symbol;
}
object* quote_symbol(void) {
	return _quote_symbol;
}
object* define_symbol(void) {
	return _define_symbol;
}
object* lambda_symbol(void) {
	return _lambda_symbol;
}
object* curry_symbol(void) {
	return _curry_symbol;
}
object* apply_symbol(void) {
	return _apply_symbol;
}
object* if_symbol(void) {
	return _if_symbol;
}
object* list_symbol(void) {
	return _list_symbol;
}

