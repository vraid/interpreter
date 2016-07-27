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
#include "list-util.h"
#include "bignums.h"
#include "sequences.h"
#include "environments.h"
#include "global-variables.h"
#include "symbols.h"

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

object is_fixnum_proc;

object* function_is_fixnum(object* args, object* cont) {
	return is_of_type(type_fixnum, args, cont);
}

object is_list_proc;

object* function_is_list(object* args, object* cont) {
	return is_of_type(type_list, args, cont);
}

object is_vector_proc;

object* function_is_vector(object* args, object* cont) {
	return is_of_type(type_vector_iterator, args, cont);
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
	object* result = boolean(one == two);
	
	return call_cont(cont, result);
}

object cons_proc;

object* function_cons(object* args, object* cont) {
	object* first;
	object* rest;
	delist_2(args, &first, &rest);
	
	if (!is_list(rest)) {
		return throw_error(cont, "cons on non-list");
	}
	
	object list_cell;
	init_list_cell(&list_cell, first, rest);
	
	return call_cont(cont, &list_cell);
}

object add_proc;

object* function_add(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, "+ on non-number");
	}
	
	object call;
	init_call(&call, &bignum_add_proc, args, cont);
	return perform_call(&call);
}

object negative_proc;

object* function_negative(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
		
	if (!is_bignum(a)) {
		return throw_error(cont, "negative on non-number");
	}
	
	object num;
	init_bignum(&num, -1 * bignum_sign(a), bignum_digits(a));
	return call_cont(cont, &num); 
}

object subtract_proc;

object* function_subtract(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, "- on non-number");
	}
	
	object call_args[2];
	init_list_2(call_args, b, a);
	object call;
	init_call(&call, &bignum_subtract_proc, call_args, cont);
	return perform_call(&call);
}

object subtract_by_proc;

object* function_subtract_by(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, "subtract-by on non-number");
	}
	
	object call;
	init_call(&call, &bignum_subtract_proc, args, cont);
	return perform_call(&call);
}

object multiply_proc;

object* function_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, "* on non-number");
	}
	
	object call;
	init_call(&call, &bignum_multiply_proc, args, cont);
	return perform_call(&call);
}

object numeric_equality_proc;

object* function_numeric_equality(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, "= on non-number");
	}
	
	return call_cont(cont, boolean(0 == compare_signed_bignums(a, b)));
}

object greater_proc;

object* function_greater(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, "> on non-number");
	}
	
	return call_cont(cont, boolean(1 == compare_signed_bignums(a, b)));
}

object greater_or_equal_proc;

object* function_greater_or_equal(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, ">= on non-number");
	}
	
	return call_cont(cont, boolean(-1 != compare_signed_bignums(a, b)));
}

object less_proc;

object* function_less(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, "< on non-number");
	}
	
	return call_cont(cont, boolean(-1 == compare_signed_bignums(a, b)));
}

object less_or_equal_proc;

object* function_less_or_equal(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_bignum(a) && is_bignum(b))) {
		return throw_error(cont, "<= on non-number");
	}
	
	return call_cont(cont, boolean(1 != compare_signed_bignums(a, b)));
}

object remainder_proc;

object* function_remainder(object* args, object* cont) {
	object* d;
	object* n;
	delist_2(args, &d, &n);
	
	if (!(is_fixnum(n) && is_fixnum(d))) {
		return throw_error(cont, "remainder on non-number");
	}
	
	if (fixnum_value(d) == 0) {
		return throw_error(cont, "remainder divisor 0");
	}
	
	long r = fixnum_value(n) % fixnum_value(d);
	object result;
	init_fixnum(&result, r);
	
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

char argnames[generic_args_max][2];
object argcells[generic_args_max];

#define primitive_max 1024
object primitive_functions[primitive_max];
object primitive_bodies[primitive_max];
int primitive_count = 0;

void bind_primitive(char* name, int arity, object* obj) {
	if (primitive_count >= primitive_max) {
		fprintf(stderr, "too many primitives\n");
		exit(0);
	}
	
	object* function = &primitive_functions[primitive_count];
	object* par = generic_arg_list[arity];
	object* body = &primitive_bodies[primitive_count];
	init_list_cell(body, obj, par);
	body->location = location_static;
	init_function(function, empty_environment(), par, body);
	function->location = location_static;
	
	add_static_binding(function, name);
	primitive_count++;
}

void bind_and_save_primitive(char* name, int arity, object* obj, object** saved) {
	*saved = &primitive_functions[primitive_count];
	bind_primitive(name, arity, obj);
}

void init_and_bind_primitive(char* name, int arity, object* obj, primitive_proc* proc) {
	init_primitive_procedure(obj, proc);
	bind_primitive(name, arity, obj);
}

void init_standard_functions(void) {
	int i;
	argnames[0][0] = 'a';
	argnames[0][1] = 0;
	generic_args[0] = make_static_symbol(argnames[0]);
	init_list_cell(&argcells[0], generic_args[0], empty_list());
	argcells[0].location = location_static;
	for (i = 1; i < generic_args_max; i++) {
		argnames[i][0] = argnames[i-1][0] + 1;
		argnames[i][1] = 0;
		generic_args[i] = make_static_symbol(argnames[i]);
		init_list_cell(&argcells[i], generic_args[i], &argcells[i-1]);
		argcells[i].location = location_static;
		generic_arg_list[i] = &argcells[i-1];
	}
	generic_arg_list[generic_args_max] = &argcells[generic_args_max]-1;
	
	init_and_bind_primitive("boolean?", 1, &is_boolean_proc, &function_is_boolean);
	init_and_bind_primitive("false?", 1, &is_false_proc, &function_is_false);
	init_and_bind_primitive("true?", 1, &is_true_proc, &function_is_true);
	init_and_bind_primitive("symbol?", 1, &is_symbol_proc, &function_is_symbol);
	init_and_bind_primitive("number?", 1, &is_fixnum_proc, &function_is_fixnum);
	init_and_bind_primitive("list?", 1, &is_list_proc, &function_is_list);
	init_and_bind_primitive("vector?", 1, &is_vector_proc, &function_is_vector);
	init_and_bind_primitive("function?", 1, &is_function_proc, &function_is_function);
	init_and_bind_primitive("identical?", 2, &is_identical_proc, &function_is_identical);
	init_and_bind_primitive("link", 2, &cons_proc, &function_cons);
	bind_primitive("append", 1, &list_append_proc);
	bind_primitive("first", 1, &first_proc);
	bind_primitive("rest", 1, &rest_proc);
	init_and_bind_primitive("+", 2, &add_proc, &function_add);
	init_and_bind_primitive("negative", 1, &negative_proc, &function_negative);
	init_and_bind_primitive("-", 2, &subtract_proc, &function_subtract);
	init_and_bind_primitive("subtract-by", 2, &subtract_by_proc, &function_subtract_by);
	init_and_bind_primitive("*", 2, &multiply_proc, &function_multiply);
	init_and_bind_primitive("=", 2, &numeric_equality_proc, &function_numeric_equality);
	init_and_bind_primitive(">", 2, &greater_proc, &function_greater);
	init_and_bind_primitive(">=", 2, &greater_or_equal_proc, &function_greater_or_equal);
	init_and_bind_primitive("<", 2, &less_proc, &function_less);
	init_and_bind_primitive("<=", 2, &less_or_equal_proc, &function_less_or_equal);
	init_and_bind_primitive("remainder", 2, &remainder_proc, &function_remainder);
	bind_primitive("identity", 1, &identity_proc);
	add_static_binding(empty_stream(), "empty-stream");
	bind_primitive("take", 2, &take_proc);
	bind_primitive("drop", 2, &drop_proc);
	bind_and_save_primitive("symbol->string", 1, &symbol_to_string_proc, &symbol_to_string_func);
}
