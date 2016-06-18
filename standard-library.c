#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "standard-library.h"
#include "data-structures.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"
#include "print.h"
#include "base-util.h"
#include "environments.h"
#include "global-variables.h"
#include "symbols.h"

object* boolean(char b) {
	return b ? true() : false();
}

object* is_of_type(object_type type, object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	obj = unquote(obj);
	object* result = boolean(is_type(type, obj));
	
	return call_cont(cont, result);
}

object* is_id(object* id, object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	obj = unquote(obj);
	object* result = boolean(obj == id);
	
	return call_cont(cont, result);
}

object is_boolean_proc;

object* function_is_boolean(object* args, object* cont) {
	return is_of_type(type_boolean, args, cont);
}

object is_false_proc;

object* function_is_false(object* args, object* cont) {
	return is_id(false(), args, cont);
}

object is_true_proc;

object* function_is_true(object* args, object* cont) {
	return is_id(true(), args, cont);
}

object is_symbol_proc;

object* function_is_symbol(object* args, object* cont) {
	return is_of_type(type_symbol, args, cont);
}

object is_number_proc;

object* function_is_number(object* args, object* cont) {
	return is_of_type(type_number, args, cont);
}

object is_list_proc;

object* function_is_list(object* args, object* cont) {
	return is_of_type(type_list, args, cont);
}

object is_function_proc;

object* function_is_function(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	object* result = boolean(is_type(type_function, obj) || is_type(type_primitive_procedure, obj));
	
	return call_cont(cont, result);
}

object is_identical_proc;

object* function_is_identical(object* args, object* cont) {
	object* one;
	object* two;
	delist_2(args, &one, &two);
	one = unquote(one);
	two = unquote(two);
	object* result = boolean(one == two);
	
	return call_cont(cont, result);
}

object identity_proc;

object* function_identity(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	return call_cont(cont, obj);
}

object cons_proc;

object* function_cons(object* args, object* cont) {
	object* first;
	object* rest;
	delist_2(args, &first, &rest);
	rest = unquote(rest);
	object list_cell;
	init_list_cell(&list_cell, first, rest);
	
	object ls[1];
	init_list_1(ls, &list_cell);
	object quote_call;
	init_call(&quote_call, quote_object_proc(), ls, cont);
	
	return perform_call(&quote_call);
}

object add_proc;

object* function_add(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	object result;
	init_number(&result, number_value(a) + number_value(b));
	return call_cont(cont, &result);
}

object negative_proc;

object* function_negative(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	object result;
	init_number(&result, -number_value(a));
	return call_cont(cont, &result);
}

object subtract_proc;

object* function_subtract(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	object result;
	init_number(&result, number_value(b) - number_value(a));
	return call_cont(cont, &result);
}

object multiply_proc;

object* function_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	object result;
	init_number(&result, number_value(a) * number_value(b));
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

#define args_max 3
char argnames[args_max][2];
object argcells[args_max];
object* args[args_max+1];

#define primitive_max 1024
object primitive_functions[primitive_max];
object primitive_bodies[primitive_max];
int primitive_count = 0;

void init_and_bind_primitive(char* name, int arity, object* obj, primitive_proc* proc) {
	if (primitive_count >= primitive_max) {
		fprintf(stderr, "too many primitives\n");
		exit(0);
	}
	
	init_primitive_procedure(obj, proc);
	
	object* function = &primitive_functions[primitive_count];
	object* par = args[arity];
	object* body = &primitive_bodies[primitive_count];
	init_list_cell(body, obj, par);
	init_function(function, empty_environment(), par, body);
	
	add_static_binding(function, name);
	primitive_count++;
}

void init_standard_functions(void) {
	int i;
	argnames[0][0] = 'a';
	argnames[0][1] = 0;
	init_list_cell(&argcells[0], make_static_symbol(argnames[0]), empty_list());
	for (i = 1; i < args_max; i++) {
		argnames[i][0] = argnames[i-1][0] + 1;
		argnames[i][1] = 0;
		init_list_cell(&argcells[i], make_static_symbol(argnames[i]), &argcells[i-1]);
		args[i] = &argcells[i-1];
	}
	args[args_max] = &argcells[args_max]-1;
	
	init_and_bind_primitive("boolean?", 1, &is_boolean_proc, &function_is_boolean);
	init_and_bind_primitive("false?", 1, &is_false_proc, &function_is_false);
	init_and_bind_primitive("true?", 1, &is_true_proc, &function_is_true);
	init_and_bind_primitive("symbol?", 1, &is_symbol_proc, &function_is_symbol);
	init_and_bind_primitive("number?", 1, &is_number_proc, &function_is_number);
	init_and_bind_primitive("list?", 1, &is_list_proc, &function_is_list);
	init_and_bind_primitive("function?", 1, &is_function_proc, &function_is_function);
	init_and_bind_primitive("identical?", 2, &is_identical_proc, &function_is_identical);
	init_and_bind_primitive("identity", 1, &identity_proc, &function_identity);
	init_and_bind_primitive("link", 2, &cons_proc, &function_cons);
	init_and_bind_primitive("+", 2, &add_proc, &function_add);
	init_and_bind_primitive("negative", 1, &negative_proc, &function_negative);
	init_and_bind_primitive("-", 2, &subtract_proc, &function_subtract);
	init_and_bind_primitive("*", 2, &multiply_proc, &function_multiply);
}
