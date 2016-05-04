#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "standard-library.h"
#include "data-structures.h"
#include "object-init.h"
#include "list-util.h"
#include "delist.h"
#include "call.h"
#include "print.h"
#include "environments.h"
#include "global-variables.h"
#include "symbols.h"

object* boolean(char b) {
	return b ? true() : false();
}

object* is_of_type(object_type type, object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	object* result = boolean(is_type(type, obj));
	
	return call_cont(cont, result);
}

object* is_id(object* id, object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	object* result = boolean(obj == id);
	
	return call_cont(cont, result);
}

object* function_is_boolean(object* args, object* cont) {
	return is_of_type(type_boolean, args, cont);
}

object* function_is_false(object* args, object* cont) {
	return is_id(false(), args, cont);
}

object* function_is_true(object* args, object* cont) {
	return is_id(true(), args, cont);
}

object* function_symbol(object* args, object* cont) {
	return is_of_type(type_symbol, args, cont);
}

object* function_number(object* args, object* cont) {
	return is_of_type(type_number, args, cont);
}

object* function_list(object* args, object* cont) {
	return is_of_type(type_list, args, cont);
}

object* function_function(object* args, object* cont) {
	return is_of_type(type_function, args, cont);
}

object* function_identical(object* args, object* cont) {
	object* one;
	object* two;
	delist_2(args, &one, &two);
	object* result = boolean(one == two);
	
	return call_cont(cont, result);
}

object* function_cons(object* args, object* cont) {
	object* first;
	object* rest;
	delist_2(args, &first, &rest);
	object list_cell;
	init_list_cell(&list_cell, first, rest);
	
	return call_cont(cont, &list_cell);
}

object* function_add(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	object result;
	// init number
	return call_cont(cont, &result);
}

object* function_negative(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	object result;
	// init number
	return call_cont(cont, &result);
}

object* function_subtract(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	object result;
	// init number
	return call_cont(cont, &result);
}

object* function_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	object result;
	// init number
	return call_cont(cont, &result);
}

object display_proc;

object* function_display(object* args, object* cont) {
	return print_value(args, cont);
}

object newline_proc;

object* function_newline(object* args, object* cont) {
	printf("\n");
	return call_cont(cont, no_object());
}


object* function_display_newline(object* args, object* cont) {
	object call;
	init_call(&call, &newline_proc, empty_list(), cont);
	object next_cont;
	init_cont(&next_cont, &call);
	return function_display(args, &next_cont);
}

void init_standard_functions(void) {
	init_primitive_procedure(&display_proc, &function_display);
	init_primitive_procedure(&newline_proc, &function_newline);
	init_primitive_procedure(&display_newline_proc, &function_display_newline);
}

object* bind_primitive(char* name, primitive_proc proc) {
	return no_object();
	// make_binding(symbol(name, NULL), make_function(empty_environment(), parameters, cons(make_primitive_procedure(proc), parameters)));
}

object* env(object* binding, object* environment) {
	return empty_environment();
	// make_environment(cons(binding, environment_bindings(environment)));
}

object* standard_environment(void) {
	object* obj = empty_environment();
	/*
	obj = env(bind_primitive("boolean?", function_boolean), obj);
	obj = env(bind_primitive("symbol?", function_symbol), obj);
	obj = env(bind_primitive("number?", function_number), obj);
	obj = env(bind_primitive("list?", function_list), obj);
	obj = env(bind_primitive("function?", function_function), obj);
	obj = env(bind_primitive("false?", function_false), obj);
	obj = env(bind_primitive("true?", function_true), obj);
	obj = env(bind_primitive("identical?", function_identical), obj);
	obj = env(bind_primitive("cons", function_cons), obj);
	obj = env(bind_primitive("negative", function_negative), obj);
	obj = env(bind_primitive("add", function_add), obj);
	obj = env(bind_primitive("subtract", function_subtract), obj);
	obj = env(bind_primitive("multiply", function_multiply), obj);
	obj = env(bind_primitive("display", function_display), obj);
	*/
	return obj;
}
