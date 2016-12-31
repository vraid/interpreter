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
#include "complex.h"
#include "numbers.h"
#include "sequences.h"
#include "environments.h"
#include "global-variables.h"
#include "symbols.h"
#include "generic-arguments.h"

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

object std_is_boolean_proc;

object* std_is_boolean(object* args, object* cont) {
	return is_of_type(type_boolean, args, cont);
}

object std_is_false_proc;

object* std_is_false(object* args, object* cont) {
	return is_id(false(), args, cont);
}

object std_is_true_proc;

object* std_is_true(object* args, object* cont) {
	return is_id(true(), args, cont);
}

object std_is_proc;

object* std_is(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	return call_cont(cont, boolean(a != false()));
}

object std_is_symbol_proc;

object* std_is_symbol(object* args, object* cont) {
	return is_of_type(type_symbol, args, cont);
}

object std_is_integer_proc;

object* std_is_integer(object* args, object* cont) {
	return is_of_type(type_integer, args, cont);
}

object std_is_fraction_proc;

object* std_is_fraction(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	return call_cont(cont, boolean(is_exact_real(a)));
}

object std_is_true_fraction_proc;

object* std_is_true_fraction(object* args, object* cont) {
	return is_of_type(type_fraction, args, cont);
}

object std_is_complex_proc;

object* std_is_complex(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	return call_cont(cont, boolean(is_exact_complex(a)));
}

object std_is_true_complex_proc;

object* std_is_true_complex(object* args, object* cont) {	
	return is_of_type(type_complex, args, cont);
}

object std_is_real_proc;

object* std_is_real(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	return call_cont(cont, boolean(is_exact_real(a)));
}

object std_is_imaginary_proc;

object* std_is_imaginary(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	return call_cont(cont, boolean(is_exact_imaginary(a)));
}

object std_is_list_proc;

object* std_is_list(object* args, object* cont) {
	return is_of_type(type_list, args, cont);
}

object std_is_vector_proc;

object* std_is_vector(object* args, object* cont) {
	return is_of_type(type_vector_iterator, args, cont);
}

object std_is_function_proc;

object* std_is_function(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	object* result = boolean(is_type(type_function, obj) || is_type(type_primitive_procedure, obj));
	
	return call_cont(cont, result);
}

object std_is_identical_proc;

object* std_is_identical(object* args, object* cont) {
	object* one;
	object* two;
	delist_2(args, &one, &two);
	object* result = boolean(one == two);
	
	return call_cont(cont, result);
}

object std_not_proc;

object* std_not(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	return call_cont(cont, boolean(a == false()));
}

object std_not_identical_proc;

object* std_not_identical(object* args, object* cont) {
	object* one;
	object* two;
	delist_2(args, &one, &two);
	object* result = boolean(one != two);
	
	return call_cont(cont, result);
}

object std_is_equivalent_proc;

// the same as identical, for now
object* std_is_equivalent(object* args, object* cont) {
	object* one;
	object* two;
	delist_2(args, &one, &two);
	object* result = boolean(one == two);
	
	return call_cont(cont, result);
}

object std_cons_proc;

object* std_cons(object* args, object* cont) {
	object* first;
	object* rest;
	delist_2(args, &first, &rest);
	
	if (!is_list(rest)) {
		return throw_error(cont, "link on non-list");
	}
	
	object list_cell;
	init_list_cell(&list_cell, first, rest);
	
	return call_cont(cont, &list_cell);
}

object std_real_proc;

object* std_real(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	if (!is_exact_real(a)) {
		return throw_error(cont, "real of non-real");
	}
	
	return call_cont(cont, a);
}

object std_imaginary_proc;

object* std_imaginary(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	if (!is_exact_real(a)) {
		return throw_error(cont, "imaginary of non-real");
	}
	
	object ls[2];
	init_list_2(ls, integer_zero(), a);
	object call;
	init_call(&call, &make_complex_proc, ls, cont);
	
	return perform_call(&call);
}

object std_complex_proc;

object* std_complex(object* args, object* cont) {
	object* real;
	object* imag;
	delist_2(args, &real, &imag);
	
	if (!(is_exact_real(real) && is_exact_real(imag))) {
		return throw_error(cont, "complex of non-real");
	}
	
	object call;
	init_call(&call, &make_complex_proc, args, cont);
	return perform_call(&call);
}

object std_complex_real_proc;

object* std_complex_real(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	if (!is_exact_complex(num)) {
		return throw_error(cont, "complex-real on non-complex");
	}
	if (is_complex(num)) {
		return call_cont(cont, complex_real_part(num));
	}
	else {
		return call_cont(cont, num);
	}
}

object std_complex_imaginary_proc;

object* std_complex_imaginary(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	if (!is_exact_complex(num)) {
		return throw_error(cont, "complex-imaginary on non-complex");
	}
	if (is_complex(num)) {
		return call_cont(cont, complex_imag_part(num));
	}
	else {
		return call_cont(cont, integer_zero());
	}
}

object std_add_proc;

object* std_add(object* args, object* cont) {
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

object std_negative_proc;

object* std_negative(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	if (!is_number(a)) {
		return throw_error(cont, "negative on non-number");
	}
	
	object call;
	init_call(&call, &number_negate_proc, args, cont);
	return perform_call(&call);
}

object std_conjugate_proc;

object* std_conjugate(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	if (!is_number(a)) {
		return throw_error(cont, "conjugate on non-number");
	}
	
	object call;
	init_call(&call, &number_conjugate_proc, args, cont);
	return perform_call(&call);
}

object std_subtract_proc;

object* std_subtract(object* args, object* cont) {
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

object std_subtract_by_proc;

object* std_subtract_by(object* args, object* cont) {
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

object std_multiply_proc;

object* std_multiply(object* args, object* cont) {
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

object std_divide_proc;

object* std_divide(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (number_is_zero(b)) {
		return throw_error(cont, "division by zero");
	}
	else if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "/ on non-number");
	}
	
	object call_args[2];
	init_list_2(call_args, b, a);
	object call;
	init_call(&call, &number_divide_proc, call_args, cont);
	return perform_call(&call);
}

object std_divide_by_proc;

object* std_divide_by(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (number_is_zero(a)) {
		return throw_error(cont, "division by zero");
	}
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "divide-by on non-number");
	}
	
	object call;
	init_call(&call, &number_divide_proc, args, cont);
	return perform_call(&call);
}

object std_quotient_proc;

object* std_quotient(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	if (!(is_integer(divisor) && is_integer(dividend))) {
		return throw_error(cont, "quotient on non-number");
	}
	if (integer_is_zero(divisor)) {
		return throw_error(cont, "division by zero (quotient)");
	}
	
	object call;
	init_call(&call, &integer_quotient_proc, args, cont);
	
	return perform_call(&call);
}

object std_remainder_proc;

object* std_remainder(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	if (!(is_integer(divisor) && is_integer(dividend))) {
		return throw_error(cont, "remainder on non-number");
	}
	if (integer_is_zero(divisor)) {
		return throw_error(cont, "division by zero (remainder)");
	}
	
	object call;
	init_call(&call, &integer_remainder_proc, args, cont);
	
	return perform_call(&call);
}

object std_modulo_continued_proc;

object* std_modulo_continued(object* args, object* cont) {
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

object std_modulo_proc;

object* std_modulo(object* args, object* cont) {
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
	init_call(&next_call, &std_modulo_continued_proc, next_args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	
	object call;
	init_call(&call, &integer_divide_proc, args, &next_cont);
	
	return perform_call(&call);
}

object std_gcd_proc;

object* std_gcd(object* args, object* cont) {
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
	init_call(&eq_call, &std_is_identical_proc, eq_args, cont);
	object eq_cont;
	init_cont(&eq_cont, &eq_call);
	
	object comp_call;
	init_call(&comp_call, &number_compare_proc, nums, &eq_cont);
	
	return perform_call(&comp_call);
}

object std_numeric_equality_proc;

object* std_numeric_equality(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_number(a) && is_number(b))) {
		return throw_error(cont, "= on non-number");
	}
	
	return compare_numbers(args, zero(), cont);
}

object std_greater_proc;

object* std_greater(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_exact_real(a) && is_exact_real(b))) {
		return throw_error(cont, "> on non-real");
	}
	
	return compare_numbers(args, one(), cont);
}

object std_less_proc;

object* std_less(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_exact_real(a) && is_exact_real(b))) {
		return throw_error(cont, "< on non-number");
	}
	
	return compare_numbers(args, negative_one(), cont);
}

object std_greater_or_equal_proc;

object* std_greater_or_equal(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_exact_real(a) && is_exact_real(b))) {
		return throw_error(cont, ">= on non-number");
	}
	
	object not_call;
	init_call(&not_call, &std_not_proc, empty_list(), cont);
	object not_cont;
	init_cont(&not_cont, &not_call);
	return std_less(args, &not_cont);
}

object std_less_or_equal_proc;

object* std_less_or_equal(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (!(is_exact_real(a) && is_exact_real(b))) {
		return throw_error(cont, "<= on non-number");
	}
	
	object not_call;
	init_call(&not_call, &std_not_proc, empty_list(), cont);
	object not_cont;
	init_cont(&not_cont, &not_call);
	return std_greater(args, &not_cont);
}

object std_is_zero_proc;

object* std_is_zero(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	if (!is_number(a)) {
		return throw_error(cont, "zero? on non-number");
	}
	
	return call_cont(cont, boolean(number_is_zero(a)));
}

object std_is_positive_proc;

object* std_is_positive(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	if (!is_exact_real(a)) {
		return throw_error(cont, "positive? on non-real");
	}
	
	return call_cont(cont, boolean(1 == real_sign(a)));
}

object std_is_negative_proc;

object* std_is_negative(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	if (!is_exact_real(a)) {
		return throw_error(cont, "negative? on non-real");
	}
	
	return call_cont(cont, boolean(-1 == real_sign(a)));
}

object std_display_proc;

object* std_display(object* args, object* cont) {
	return print_value(args, cont);
}

object std_newline_proc;

object* std_newline(object* args, object* cont) {
	printf("\n");
	return call_cont(cont, no_object());
}

object* std_display_newline(object* args, object* cont) {
	object call;
	init_call(&call, &std_newline_proc, empty_list(), cont);
	object next_cont;
	init_cont(&next_cont, &call);
	return std_display(args, &next_cont);
}

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
	init_and_bind_primitive("boolean?", 1, &std_is_boolean, &std_is_boolean_proc);
	init_and_bind_primitive("false?", 1, &std_is_false, &std_is_false_proc);
	init_and_bind_primitive("true?", 1, &std_is_true, &std_is_true_proc);
	init_and_bind_primitive("is?", 1, &std_is, &std_is_proc);
	init_and_bind_primitive("symbol?", 1, &std_is_symbol, &std_is_symbol_proc);
	init_and_bind_primitive("integer?", 1, &std_is_integer, &std_is_integer_proc);
	init_and_bind_primitive("fraction?", 1, &std_is_fraction, &std_is_fraction_proc);
	init_and_bind_primitive("true-fraction?", 1, &std_is_true_fraction, &std_is_true_fraction_proc);
	init_and_bind_primitive("complex?", 1, &std_is_complex, &std_is_complex_proc);
	init_and_bind_primitive("true-complex?", 1, &std_is_true_complex, &std_is_true_complex_proc);
	init_and_bind_primitive("real?", 1, &std_is_real, &std_is_real_proc);
	init_and_bind_primitive("imaginary?", 1, &std_is_imaginary, &std_is_imaginary_proc);
	bind_primitive("number?", 1, &std_is_complex_proc);
	init_and_bind_primitive("list?", 1, &std_is_list, &std_is_list_proc);
	init_and_bind_primitive("vector?", 1, &std_is_vector, &std_is_vector_proc);
	init_and_bind_primitive("function?", 1, &std_is_function, &std_is_function_proc);
	init_and_bind_primitive("same?", 2, &std_is_identical, &std_is_identical_proc);
	init_and_bind_primitive("eq?", 2, &std_is_equivalent, &std_is_equivalent_proc);
	init_and_bind_primitive("not", 1, &std_not, &std_not_proc);
	init_primitive(&std_not_identical, &std_not_identical_proc);
	init_and_bind_primitive("link", 2, &std_cons, &std_cons_proc);
	bind_primitive("append", 1, &list_append_proc);
	bind_primitive("first", 1, &first_proc);
	bind_primitive("rest", 1, &rest_proc);
	init_and_bind_primitive("zero?", 1, &std_is_zero, &std_is_zero_proc);
	init_and_bind_primitive("positive?", 1, &std_is_positive, &std_is_positive_proc);
	init_and_bind_primitive("negative?", 1, &std_is_negative, &std_is_negative_proc);
	init_and_bind_primitive("real", 1, &std_real, &std_real_proc);
	init_and_bind_primitive("imaginary", 1, &std_imaginary, &std_imaginary_proc);
	init_and_bind_primitive("complex", 2, &std_complex, &std_complex_proc);
	init_and_bind_primitive("complex-real", 1, &std_complex_real, &std_complex_real_proc);
	init_and_bind_primitive("complex-imaginary", 1, &std_complex_imaginary, &std_complex_imaginary_proc);
	init_and_bind_primitive("+", 2, &std_add, &std_add_proc);
	init_and_bind_primitive("negative", 1, &std_negative, &std_negative_proc);
	init_and_bind_primitive("conjugate", 1, &std_conjugate, &std_conjugate_proc);
	init_and_bind_primitive("-", 2, &std_subtract, &std_subtract_proc);
	init_and_bind_primitive("subtract-by", 2, &std_subtract_by, &std_subtract_by_proc);
	init_and_bind_primitive("*", 2, &std_multiply, &std_multiply_proc);
	init_and_bind_primitive("/", 2, &std_divide, &std_divide_proc);
	init_and_bind_primitive("divide-by", 2, &std_divide_by, &std_divide_by_proc);
	init_and_bind_primitive("quotient", 2, &std_quotient, &std_quotient_proc);
	init_and_bind_primitive("remainder", 2, &std_remainder, &std_remainder_proc);
	init_and_bind_primitive("modulo", 2, &std_modulo, &std_modulo_proc);
	init_primitive(&std_modulo_continued, &std_modulo_continued_proc);
	init_and_bind_primitive("gcd", 2, &std_gcd, &std_gcd_proc);
	init_and_bind_primitive("=", 2, &std_numeric_equality, &std_numeric_equality_proc);
	init_and_bind_primitive(">", 2, &std_greater, &std_greater_proc);
	init_and_bind_primitive(">=", 2, &std_greater_or_equal, &std_greater_or_equal_proc);
	init_and_bind_primitive("<", 2, &std_less, &std_less_proc);
	init_and_bind_primitive("<=", 2, &std_less_or_equal, &std_less_or_equal_proc);
	bind_primitive("identity", 1, &identity_proc);
	add_static_binding(empty_stream(), "empty-stream");
	bind_primitive("take", 2, &take_proc);
	bind_primitive("drop", 2, &drop_proc);
	bind_and_save_primitive("symbol->string", 1, &symbol_to_string_func, &symbol_to_string_proc);
}
