#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "standard-library.h"
#include "data-structures.h"
#include "list-util.h"
#include "environments.h"
#include "global-variables.h"
#include "symbols.h"

/*

typedef struct arg2 {
	object* first;
	object* second;
} arg2;

arg2 args2(object* arguments) {
	arg2 arg;
	arg.first = list_first(arguments);
	arguments = list_rest(arguments);
	arg.second = list_first(arguments);
	return arg;
}

object* boolean(char b) {
	return b ? true() : false();
}

object* is_of_type(object_type type, object* arguments) {
	return boolean(is_type(type, list_first(arguments)));
}

object* function_boolean(object* arguments, object* cont) {
	return is_of_type(type_boolean, list_first(arguments));
}

object* function_false(object* arguments, object* cont) {
	return boolean(is_false(list_first(arguments)));
}

object* function_true(object* arguments, object* cont) {
	return boolean(is_true(list_first(arguments)));
}

object* function_symbol(object* arguments, object* cont) {
	return is_of_type(type_symbol, list_first(arguments));
}

object* function_number(object* arguments, object* cont) {
	return is_of_type(type_number, list_first(arguments));
}

object* function_list(object* arguments, object* cont) {
	return is_of_type(type_list, list_first(arguments));
}

object* function_function(object* arguments, object* cont) {
	return is_of_type(type_function, list_first(arguments));
}

object* function_identical(object* arguments, object* cont) {
	arg2 args = args2(arguments);
	return boolean(args.first == args.second);
}

object* function_cons(object* arguments, object* cont) {
	arg2 args = args2(arguments);
	return cons(args.first, args.second);
}

object* function_add(object* arguments, object* cont) {
	arg2 args = args2(arguments);
	return make_number(number_value(args.first) + number_value(args.second));
}

object* function_negative(object* arguments, object* cont) {
	return make_number(-number_value(list_first(arguments)));
}

object* function_subtract(object* arguments, object* cont) {
	arg2 args = args2(arguments);
	return make_number(-number_value(args.first) + number_value(args.second));
}

object* function_multiply(object* arguments, object* cont) {
	arg2 args = args2(arguments);
	return make_number(number_value(args.first) * number_value(args.second));
}

object* function_display(object* arguments) {
	printf("display\n");
}

object* bind_primitive(char* name, primitive_proc proc) {
	return no_object();
	// make_binding(symbol(name, NULL), make_function(empty_environment(), parameters, cons(make_primitive_procedure(proc), parameters)));
}

object* env(object* binding, object* environment) {
	return empty_environment();
	// make_environment(cons(binding, environment_bindings(environment)));
}

*/

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
