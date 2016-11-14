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
#include "integers.h"
#include "fractions.h"
#include "numbers.h"
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

object is_integer_proc;

object* function_is_integer(object* args, object* cont) {
	return is_of_type(type_integer, args, cont);
}

object is_fraction_proc;

object* function_is_fraction(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	return call_cont(cont, boolean(is_exact_real(a)));
}

object is_true_fraction_proc;

object* function_is_true_fraction(object* args, object* cont) {
	return is_of_type(type_fraction, args, cont);
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

object not_proc;

object* function_not(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	return call_cont(cont, boolean(a == false()));
}

object not_identical_proc;

object* not_identical(object* args, object* cont) {
	object* one;
	object* two;
	delist_2(args, &one, &two);
	object* result = boolean(one != two);
	
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
	
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "+ on non-number");
	}
	
	object call;
	init_call(&call, &number_add_proc, args, cont);
	return perform_call(&call);
}

object negative_proc;

object* function_negative(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	if (is_integer(a)) {
		object num;
		init_integer(&num, - 1 * integer_sign(a), integer_digits(a));
		return call_cont(cont, &num);
	}
	else if (is_fraction(a)) {
		object* numerator = fraction_numerator(a);
		
		object num;
		init_integer(&num, -1 * integer_sign(numerator), integer_digits(numerator));
		
		object fraction;
		init_fraction(&fraction, &num, fraction_denominator(a));
		
		return call_cont(cont, &fraction);
	}
	else {
		return throw_error(cont, "negative on non-number");
	}
}

object subtract_proc;

object* function_subtract(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "- on non-number");
	}
	
	object call_args[2];
	init_list_2(call_args, b, a);
	object call;
	init_call(&call, &number_subtract_proc, call_args, cont);
	return perform_call(&call);
}

object subtract_by_proc;

object* function_subtract_by(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "subtract-by on non-number");
	}
	
	object call;
	init_call(&call, &number_subtract_proc, args, cont);
	return perform_call(&call);
}

object multiply_proc;

object* function_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "* on non-number");
	}
	
	object call;
	init_call(&call, &number_multiply_proc, args, cont);
	return perform_call(&call);
}

object divide_proc;

object* function_divide(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "/ on non-number");
	}
	
	object call_args[2];
	init_list_2(call_args, b, a);
	object call;
	init_call(&call, &number_divide_proc, call_args, cont);
	return perform_call(&call);
}

object divide_by_proc;

object* function_divide_by(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "divide-by on non-number");
	}
	
	object call;
	init_call(&call, &number_divide_proc, args, cont);
	return perform_call(&call);
}

object quotient_proc;

object* function_quotient(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	if (!(is_integer(divisor) && is_integer(dividend))) {
		return throw_error(cont, "quotient on non-number");
	}
	if (is_zero_integer(divisor)) {
		return throw_error(cont, "division by zero (quotient)");
	}
	
	object call;
	init_call(&call, &integer_quotient_proc, args, cont);
	
	return perform_call(&call);
}

object remainder_proc;

object* function_remainder(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	if (!(is_integer(divisor) && is_integer(dividend))) {
		return throw_error(cont, "remainder on non-number");
	}
	if (is_zero_integer(divisor)) {
		return throw_error(cont, "division by zero (remainder)");
	}
	
	object call;
	init_call(&call, &integer_remainder_proc, args, cont);
	
	return perform_call(&call);
}

object modulo_continued_proc;

object* modulo_continued(object* args, object* cont) {
	object* ls;
	object* divisor;
	delist_2(args, &ls, &divisor);
	
	object* quotient;
	object* remainder;
	delist_2(ls, &quotient, &remainder);
	
	if (is_negative(remainder)) {
		object add_args[2];
		init_list_2(add_args, remainder, divisor);
		object add_call;
		init_call(&add_call, &integer_add_proc, add_args, cont);
		
		return perform_call(&add_call);
	}
	else {
		return call_cont(cont, remainder);
	}
}

object modulo_proc;

object* function_modulo(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	if (!(is_integer(divisor) && is_integer(dividend))) {
		return throw_error(cont, "modulo on non-integer");
	}
	else if (!is_positive(divisor)) {
		return throw_error(cont, "modulo with nonpositive base");
	}
	
	object next_args[1];
	init_list_1(next_args, divisor);
	object next_call;
	init_call(&next_call, &modulo_continued_proc, next_args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	
	object call;
	init_call(&call, &integer_divide_proc, args, &next_cont);
	
	return perform_call(&call);
}

object gcd_proc;

object* function_gcd(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_integer(a) && is_integer(b))) {
		return throw_error(cont, "gcd on non-integer");
	}
	
	object gcd_call;
	init_call(&gcd_call, &integer_greatest_common_divisor_proc, args, cont);
	
	return perform_call(&gcd_call);
}

object* compare_numbers(object* nums, object* result, object* cont) {
	object eq_args[1];
	init_list_1(eq_args, result);
	object eq_call;
	init_call(&eq_call, &is_identical_proc, eq_args, cont);
	object eq_cont;
	init_cont(&eq_cont, &eq_call);
	
	object comp_call;
	init_call(&comp_call, &number_compare_proc, nums, &eq_cont);
	
	return perform_call(&comp_call);
}

object numeric_equality_proc;

object* function_numeric_equality(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "= on non-number");
	}
	
	return compare_numbers(args, zero(), cont);
}

object greater_proc;

object* function_greater(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_exact_real(a) && is_exact_real(b))) {
		return throw_error(cont, "> on non-real");
	}
	
	return compare_numbers(args, one(), cont);
}

object less_proc;

object* function_less(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_exact_real(a) && is_exact_real(b))) {
		return throw_error(cont, "< on non-number");
	}
	
	return compare_numbers(args, negative_one(), cont);
}

object greater_or_equal_proc;

object* function_greater_or_equal(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_exact_real(a) && is_exact_real(b))) {
		return throw_error(cont, ">= on non-number");
	}
	
	object not_call;
	init_call(&not_call, &not_proc, empty_list(), cont);
	object not_cont;
	init_cont(&not_cont, &not_call);
	return function_less(args, &not_cont);
}

object less_or_equal_proc;

object* function_less_or_equal(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_exact_real(a) && is_exact_real(b))) {
		return throw_error(cont, "<= on non-number");
	}
	
	object not_call;
	init_call(&not_call, &not_proc, empty_list(), cont);
	object not_cont;
	init_cont(&not_cont, &not_call);
	return function_greater(args, &not_cont);
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
	make_static(body);
	init_function(function, empty_environment(), par, body);
	make_static(function);
	
	add_static_binding(function, name);
	primitive_count++;
}

void bind_and_save_primitive(char* name, int arity, object** saved, object* obj) {
	*saved = &primitive_functions[primitive_count];
	bind_primitive(name, arity, obj);
}

void init_and_bind_primitive(char* name, int arity, primitive_proc* proc, object* obj) {
	init_primitive(proc, obj);
	bind_primitive(name, arity, obj);
}

void init_standard_functions(void) {
	int i;
	argnames[0][0] = 'a';
	argnames[0][1] = 0;
	generic_args[0] = make_static_symbol(argnames[0]);
	init_list_cell(&argcells[0], generic_args[0], empty_list());
	make_static(&argcells[0]);
	for (i = 1; i < generic_args_max; i++) {
		argnames[i][0] = argnames[i-1][0] + 1;
		argnames[i][1] = 0;
		generic_args[i] = make_static_symbol(argnames[i]);
		init_list_cell(&argcells[i], generic_args[i], &argcells[i-1]);
		make_static(&argcells[i]);
		generic_arg_list[i] = &argcells[i-1];
	}
	generic_arg_list[generic_args_max] = &argcells[generic_args_max]-1;
	
	init_and_bind_primitive("boolean?", 1, &function_is_boolean, &is_boolean_proc);
	init_and_bind_primitive("false?", 1, &function_is_false, &is_false_proc);
	init_and_bind_primitive("true?", 1, &function_is_true, &is_true_proc);
	init_and_bind_primitive("symbol?", 1, &function_is_symbol, &is_symbol_proc);
	init_and_bind_primitive("integer?", 1, &function_is_integer, &is_integer_proc);
	init_and_bind_primitive("fraction?", 1, &function_is_fraction, &is_fraction_proc);
	init_and_bind_primitive("true-fraction?", 1, &function_is_true_fraction, &is_true_fraction_proc);
	bind_primitive("number?", 1, &is_fraction_proc);
	init_and_bind_primitive("list?", 1, &function_is_list, &is_list_proc);
	init_and_bind_primitive("vector?", 1, &function_is_vector, &is_vector_proc);
	init_and_bind_primitive("function?", 1, &function_is_function, &is_function_proc);
	init_and_bind_primitive("identical?", 2, &function_is_identical, &is_identical_proc);
	init_and_bind_primitive("not", 1, &function_not, &not_proc);
	init_primitive(&not_identical, &not_identical_proc);
	init_and_bind_primitive("link", 2, &function_cons, &cons_proc);
	bind_primitive("append", 1, &list_append_proc);
	bind_primitive("first", 1, &first_proc);
	bind_primitive("rest", 1, &rest_proc);
	init_and_bind_primitive("+", 2, &function_add, &add_proc);
	init_and_bind_primitive("negative", 1, &function_negative, &negative_proc);
	init_and_bind_primitive("-", 2, &function_subtract, &subtract_proc);
	init_and_bind_primitive("subtract-by", 2, &function_subtract_by, &subtract_by_proc);
	init_and_bind_primitive("*", 2, &function_multiply, &multiply_proc);
	init_and_bind_primitive("/", 2, &function_divide, &divide_proc);
	init_and_bind_primitive("divide-by", 2, &function_divide_by, &divide_by_proc);
	init_and_bind_primitive("quotient", 2, &function_quotient, &quotient_proc);
	init_and_bind_primitive("remainder", 2, &function_remainder, &remainder_proc);
	init_and_bind_primitive("modulo", 2, &function_modulo, &modulo_proc);
	init_primitive(&modulo_continued, &modulo_continued_proc);
	init_and_bind_primitive("gcd", 2, &function_gcd, &gcd_proc);
	init_and_bind_primitive("=", 2, &function_numeric_equality, &numeric_equality_proc);
	init_and_bind_primitive(">", 2, &function_greater, &greater_proc);
	init_and_bind_primitive(">=", 2, &function_greater_or_equal, &greater_or_equal_proc);
	init_and_bind_primitive("<", 2, &function_less, &less_proc);
	init_and_bind_primitive("<=", 2, &function_less_or_equal, &less_or_equal_proc);
	bind_primitive("identity", 1, &identity_proc);
	add_static_binding(empty_stream(), "empty-stream");
	bind_primitive("take", 2, &take_proc);
	bind_primitive("drop", 2, &drop_proc);
	bind_and_save_primitive("symbol->string", 1, &symbol_to_string_func, &symbol_to_string_proc);
}
