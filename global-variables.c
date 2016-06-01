#include <stdlib.h>
#include <string.h>

#include "global-variables.h"
#include "data-structures.h"
#include "symbols.h"
#include "object-init.h"

object _false;
object _true;
object _no_object;
object _no_symbol;
object _no_binding;
object _empty_list;
object _empty_environment;
object* _quote_symbol;
object* _define_symbol;
object* _lambda_symbol;
object* _curry_symbol;
object* _apply_symbol;
object* _if_symbol;
object* _list_symbol;
object* _map_symbol;
object* _fold_symbol;
object* _filter_symbol;

void init_boolean(object* obj, char value) {
	init_object(location_static, type_boolean, obj);
	obj->data.boolean.value = value;
}

void init_global_variables(void) {
	init_boolean(false(), 0);
	init_boolean(true(), 1);
	
	init_object(location_static, type_none, no_object());
	init_object(location_static, type_symbol, no_symbol());
	_no_symbol.data.symbol.name = malloc(sizeof(char));
	_no_symbol.data.symbol.name[0] = 0;
	
	init_object(location_static, type_binding, no_binding());
	_no_binding.data.binding.name = no_symbol();
	_no_binding.data.binding.value = no_object();
	
	init_object(location_static, type_list, empty_list());
	_empty_list.data.list.type = shapeless;
	_empty_list.data.list.first = no_object();
	_empty_list.data.list.rest = no_object();
	
	init_object(location_static, type_environment, &_empty_environment);
	_empty_environment.data.environment.bindings = empty_list();
	
	_quote_symbol = make_static_symbol("quote");
	_define_symbol = make_static_symbol("define");
	_lambda_symbol = make_static_symbol("lambda");
	_curry_symbol = make_static_symbol("curry");
	_apply_symbol = make_static_symbol("apply");
	_if_symbol = make_static_symbol("if");
	_list_symbol = make_static_symbol("list");
	_map_symbol = make_static_symbol("map");
	_fold_symbol = make_static_symbol("fold");
	_filter_symbol = make_static_symbol("filter");
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
char is_quote_symbol(object* obj) {
	return obj == quote_symbol();
}
char is_define_symbol(object* obj) {
	return obj == define_symbol();
}
char is_lambda_symbol(object* obj) {
	return obj == lambda_symbol();
}
char is_curry_symbol(object* obj) {
	return obj == curry_symbol();
}
char is_apply_symbol(object* obj) {
	return obj == apply_symbol();
}
char is_if_symbol(object* obj) {
	return obj == if_symbol();
}
char is_list_symbol(object* obj) {
	return obj == list_symbol();
}
char is_map_symbol(object* obj) {
	return obj == map_symbol();
}
char is_fold_symbol(object* obj) {
	return obj == fold_symbol();
}
char is_filter_symbol(object* obj) {
	return obj == filter_symbol();
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
object* map_symbol(void) {
	return _map_symbol;
}
object* fold_symbol(void) {
	return _fold_symbol;
}
object* filter_symbol(void) {
	return _filter_symbol;
}
