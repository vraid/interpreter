#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "standard-library.h"
#include "data-structures.h"
#include "list-util.h"
#include "environments.h"
#include "global-variables.h"

typedef struct arg2 {
	object* one;
	object* two;
} arg2;

arg2 args2(object* arguments) {
	arg2 arg;
	arg.one = list_first(arguments);
	arg.two = list_ref(1, arguments);
	return arg;
}

object* boolean(char b) {
	if (b) {
		return true();
	}
	else {
		return false();
	}
}

object* is_of_type(object_type type, object* arguments) {
	return boolean(is_type(type, list_first(arguments)));
}

object* function_boolean(object* arguments) {
	return is_of_type(type_boolean, arguments);
}

object* function_false(object* arguments) {
	return boolean(is_false(list_first(arguments)));
}

object* function_true(object* arguments) {
	return boolean(is_true(list_first(arguments)));
}

object* function_symbol(object* arguments) {
	return is_of_type(type_symbol, arguments);
}

object* function_number(object* arguments) {
	return is_of_type(type_number, arguments);
}

object* function_list(object* arguments) {
	return is_of_type(type_list, arguments);
}

object* function_function(object* arguments) {
	return is_of_type(type_function, arguments);
}

object* function_identical(object* arguments) {
	arg2 args = args2(arguments);
	if (args.one == args.two) {
		return true();
	}
	else {
		return false();
	}
}

object* function_cons(object* arguments) {
	arg2 args = args2(arguments);
	return cons(args.one, args.two);
}

object* function_add(object* arguments) {
	arg2 args = args2(arguments);
	return make_number(number_value(args.one) + number_value(args.two));
}

object* function_negative(object* arguments) {
	return make_number(-number_value(list_first(arguments)));
}

object* function_subtract(object* arguments) {
	arg2 args = args2(arguments);
	return make_number(-number_value(args.one) + number_value(args.two));
}

object* function_multiply(object* arguments) {
	arg2 args = args2(arguments);
	return make_number(number_value(args.one) * number_value(args.two));
}

object* bind_primitive(char* name, object* parameters, primitive_proc proc) {
	return make_binding(add_symbol(name), make_function(empty_environment(), parameters, cons(make_primitive_procedure(parameters, proc), parameters)));
}

object* p(char* name, object* obj) {
	return obj = cons(add_symbol(name), obj);
}

object* q(char* name) {
	return p(name, empty_list());
}

object* env(object* binding, object* environment) {
	return make_environment(cons(binding, environment_bindings(environment)));
}

object* standard_environment(void) {
	object* obj = empty_environment();
	obj = env(bind_primitive("boolean?", q("a"), function_boolean), obj);
	obj = env(bind_primitive("symbol?", q("a"), function_symbol), obj);
	obj = env(bind_primitive("number?", q("a"), function_number), obj);
	obj = env(bind_primitive("list?", q("a"), function_list), obj);
	obj = env(bind_primitive("function?", q("a"), function_function), obj);
	obj = env(bind_primitive("false?", q("a"), function_false), obj);
	obj = env(bind_primitive("true?", q("a"), function_true), obj);
	obj = env(bind_primitive("identical?", p("a", q("b")), function_identical), obj);
	obj = env(bind_primitive("cons", p("a", q("b")), function_cons), obj);
	obj = env(bind_primitive("negative", q("a"), function_negative), obj);
	obj = env(bind_primitive("add", p("a", q("b")), function_add), obj);
	obj = env(bind_primitive("subtract", p("a", q("b")), function_subtract), obj);
	obj = env(bind_primitive("multiply", p("a", q("b")), function_multiply), obj);
	return obj;
}
