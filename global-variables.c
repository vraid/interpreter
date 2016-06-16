#include <stdlib.h>
#include <string.h>

#include "global-variables.h"
#include "data-structures.h"
#include "symbols.h"
#include "object-init.h"

char nullchar;
object _false;
object _true;
object _no_object;
object _no_symbol;
object _no_binding;
object _empty_list;
object _empty_string;
object _empty_environment;
object* _lambda_symbol;

void init_boolean(object* obj, char value) {
	init_object(location_static, type_boolean, obj);
	obj->data.boolean.value = value;
}

void init_global_variables(void) {
	init_boolean(false(), 0);
	init_boolean(true(), 1);
	
	init_object(location_static, type_none, no_object());
	init_object(location_static, type_symbol, no_symbol());
	_no_symbol.data.symbol.name = empty_string();
	
	init_object(location_static, type_binding, no_binding());
	_no_binding.data.binding.name = no_symbol();
	_no_binding.data.binding.value = no_object();
	
	init_object(location_static, type_list, empty_list());
	_empty_list.data.list.type = shapeless;
	_empty_list.data.list.first = no_object();
	_empty_list.data.list.rest = no_object();
	
	nullchar = 0;
	init_object(location_static, type_string, empty_string());
	_empty_string.data.string.value = &nullchar;
	_empty_string.data.string.length = 0;
	
	init_object(location_static, type_environment, &_empty_environment);
	_empty_environment.data.environment.bindings = empty_list();
	
	_lambda_symbol = make_static_symbol("lambda");
}

object* true(void) {
	return &_true;
}
object* false(void) {
	return &_false;
}
char is_empty_list(object* obj) {
	return obj == empty_list();
}
char is_false(object* obj) {
	return obj == false();
}
char is_true(object* obj) {
	return obj == true();
}
char is_no_object(object* obj) {
	return obj == no_object();
}
object* empty_list(void) {
	return &_empty_list;
}
object* empty_string(void) {
	return &_empty_string;
}
object* no_object(void) {
	return &_no_object;
}
object* no_symbol(void) {
	return &_no_symbol;
}
object* no_binding(void) {
	return &_no_binding;
}
object* empty_environment(void) {
	return &_empty_environment;
}

object* lambda_symbol(void) {
	return _lambda_symbol;
}
