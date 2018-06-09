#include "fractions.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"
#include "list-util.h"
#include "eval.h"
#include "integers.h"

int fraction_sign(object* obj) {
	check_type(type_fraction, obj);
	return integer_sign(fraction_numerator(obj));
}

object* make_fraction(object* args, object* cont) {
	object* numerator;
	object* denominator;
	delist_2(args, &numerator, &denominator);
	
	if (integer_is_zero(numerator)) {
		return call_cont(cont, integer_zero());
	}
	else if (integer_is_one(denominator)) {
		return call_cont(cont, numerator);
	}
	else {
		object* fraction = alloc_fraction(numerator, denominator);
		
		return call_cont(cont, fraction);
	}
}

object* make_integral_fraction(object* args, object* cont) {
	object* numerator;
	delist_1(args, &numerator);
	
	object* fraction = alloc_integral_fraction(numerator);
	
	return call_cont(cont, fraction);
}

object fraction_reduce_two_proc;

object* fraction_reduce_two(object* args, object* cont) {
	object* gcd;
	object* numerator;
	object* denominator;
	delist_3(args, &gcd, &numerator, &denominator);
	
	object* num_list = alloc_list_3(&integer_quotient_proc, gcd, numerator);
	object* denom_list = alloc_list_3(&integer_quotient_proc, gcd, denominator);
	object* make_list = alloc_list_3(&make_fraction_proc, num_list, denom_list);
	
	object* eval_args = alloc_list_3(empty_environment(), make_list, false());
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object fraction_reduce_proc;

object* fraction_reduce(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	object* numerator = fraction_numerator(num);
	object* denominator = fraction_denominator(num);

	object* reduce_args = alloc_list_2(numerator, denominator);
	object* reduce_call = alloc_call(&fraction_reduce_two_proc, reduce_args, cont);
	object* reduce_cont = alloc_cont(reduce_call);
	
	object* gcd_args = alloc_list_2(numerator, denominator);
	object* gcd_call = alloc_call(&integer_greatest_common_divisor_proc, gcd_args, reduce_cont);
	
	return perform_call(gcd_call);
}

object* fraction_make_and_reduce(object* args, object* cont) {
	object* numerator;
	object* denominator;
	delist_2(args, &numerator, &denominator);
	
	if (integer_is_zero(numerator)) {
		return call_cont(cont, numerator);
	}
	
	int sign = integer_sign(numerator) * integer_sign(denominator);
	
	object* num = alloc_integer(sign, integer_digits(numerator));
	object* denom = alloc_integer(1, integer_digits(denominator));
	object* frac = alloc_fraction(num, denom);
	
	object* reduce_args = alloc_list_1(frac);
	object* reduce_call = alloc_call(&fraction_reduce_proc, reduce_args, cont);
	
	return perform_call(reduce_call);
}

object* fraction_add(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* a_num = fraction_numerator(a);
	object* a_denom = fraction_denominator(a);
	object* b_num = fraction_numerator(b);
	object* b_denom = fraction_denominator(b);
	
	object* a_num_list = alloc_list_3(&integer_multiply_proc, a_num, b_denom);
	object* b_num_list = alloc_list_3(&integer_multiply_proc, b_num, a_denom);
	object* add_list = alloc_list_3(&integer_add_proc, a_num_list, b_num_list);
	object* denom_list = alloc_list_3(&integer_multiply_proc, a_denom, b_denom);
	object* reduce_list = alloc_list_3(&fraction_make_and_reduce_proc, add_list, denom_list);
	
	object* eval_args = alloc_list_3(empty_environment(), reduce_list, false());
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object* fraction_subtract(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* a_num = fraction_numerator(a);
	object* a_denom = fraction_denominator(a);
	object* b_num = fraction_numerator(b);
	object* b_denom = fraction_denominator(b);
	
	object* a_num_list = alloc_list_3(&integer_multiply_proc, a_num, b_denom);
	object* b_num_list = alloc_list_3(&integer_multiply_proc, b_num, a_denom);
	object* subtract_list = alloc_list_3(&integer_subtract_proc, a_num_list, b_num_list);
	object* denom_list = alloc_list_3(&integer_multiply_proc, a_denom, b_denom);
	object* reduce_list = alloc_list_3(&fraction_make_and_reduce_proc, subtract_list, denom_list);
	
	object* eval_args = alloc_list_3(empty_environment(), reduce_list, false());
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object* fraction_negate(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	object* n = alloc_negated_integer(fraction_numerator(a));
	object* num = alloc_fraction(n, fraction_denominator(a));
	return call_cont(cont, num);
}

object* fraction_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* a_num = fraction_numerator(a);
	object* a_denom = fraction_denominator(a);
	object* b_num = fraction_numerator(b);
	object* b_denom = fraction_denominator(b);
	
	object* num_list = alloc_list_3(&integer_multiply_proc, a_num, b_num);
	object* denom_list = alloc_list_3(&integer_multiply_proc, a_denom, b_denom);
	object* reduce_list = alloc_list_3(&fraction_make_and_reduce_proc, num_list, denom_list);
	
	object* eval_args = alloc_list_3(empty_environment(), reduce_list, false());
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object* fraction_divide(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* a_num = fraction_numerator(a);
	object* a_denom = fraction_denominator(a);
	object* b_num = fraction_numerator(b);
	object* b_denom = fraction_denominator(b);
	
	object* num_list = alloc_list_3(&integer_multiply_proc, b_num, a_denom);
	object* denom_list = alloc_list_3(&integer_multiply_proc, b_denom, a_num);
	object* reduce_list = alloc_list_3(&fraction_make_and_reduce_proc, num_list, denom_list);
	
	object* eval_args = alloc_list_3(empty_environment(), reduce_list, false());
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object fraction_quotient_remainder_three_proc;

object* fraction_quotient_remainder_three(object* args, object* cont) {
	object* quot_rem;
	object* denominator_product;
	delist_2(args, &quot_rem, &denominator_product);
	
	object* quotient;
	object* remainder;
	delist_2(quot_rem, &quotient, &remainder);
	
	object* quot_list = alloc_list_2(&make_integer_proc, quotient);
	object* rem_list = alloc_list_3(&fraction_make_and_reduce_proc, remainder, denominator_product);
	object* result_list = alloc_list_2(quot_list, rem_list);
	
	object* eval_args = alloc_list_3(result_list, empty_environment(), false());
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object fraction_quotient_remainder_two_proc;

object* fraction_quotient_remainder_two(object* args, object* cont) {
	object* denominator_product;
	object* a;
	object* b;
	delist_3(args, &denominator_product, &a, &b);
	
	object* a_num = fraction_numerator(a);
	object* a_denom = fraction_denominator(a);
	object* b_num = fraction_numerator(b);
	object* b_denom = fraction_denominator(b);
	
	object* next_args = alloc_list_1(denominator_product);
	object* next_call = alloc_call(&fraction_quotient_remainder_three_proc, next_args, cont);
	object* next_cont = alloc_cont(next_call);
	
	object* a_list = alloc_list_3(&integer_multiply_proc, a_num, b_denom);
	object* b_list = alloc_list_3(&integer_multiply_proc, b_num, a_denom);
	object* divide_list = alloc_list_3(&integer_divide_proc, a_list, b_list);
	
	object* eval_args = alloc_list_3(empty_environment(), divide_list, false());
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, next_cont);
	
	return perform_call(eval_call);
}

object fraction_quotient_remainder_proc;

object* fraction_quotient_remainder(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* a_denom = fraction_denominator(a);
	object* b_denom = fraction_denominator(b);
	
	object* next_call = alloc_call(&fraction_quotient_remainder_two_proc, args, cont);
	object* next_cont = alloc_cont(next_call);
	
	object* multiply_args = alloc_list_2(a_denom, b_denom);
	object* multiply_call = alloc_call(&integer_multiply_proc, multiply_args, next_cont);
	
	return perform_call(multiply_call);
}

object fraction_quotient_continued_proc;

object* fraction_quotient_continued(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	return call_cont(cont, list_first(ls));
}

object* fraction_quotient(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	object* next_call = alloc_call(&fraction_quotient_continued_proc, empty_list(), cont);
	object* next_cont = alloc_cont(next_call);
	
	object* call = alloc_call(&fraction_quotient_remainder_proc, args, next_cont);
	
	return perform_call(call);
}

object fraction_remainder_continued_proc;

object* fraction_remainder_continued(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	return call_cont(cont, list_ref(1, ls));
}

object* fraction_remainder(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	object* next_call = alloc_call(&fraction_remainder_continued_proc, empty_list(), cont);
	object* next_cont = alloc_cont(next_call);
	
	object* call = alloc_call(&fraction_quotient_remainder_proc, args, next_cont);
	
	return perform_call(call);
}

object* fraction_compare(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* a_num = fraction_numerator(a);
	object* b_num = fraction_numerator(b);
	
	if (integers_have_different_signs(a_num, b_num)) {
		return call_cont(cont, (integer_sign(a_num) == 1) ? one() : negative_one());
	}
	else {
		object* a_list = alloc_list_3(&integer_multiply_proc, a_num, fraction_denominator(b));
		object* b_list = alloc_list_3(&integer_multiply_proc, b_num, fraction_denominator(a));
		object* compare_list = alloc_list_3(&integer_compare_proc, a_list, b_list);
		
		object* eval_args = alloc_list_3(empty_environment(), compare_list, false());
		object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
		
		return perform_call(eval_call);
	}
}

void init_fraction_procedures(void) {
	init_primitive(&make_fraction, &make_fraction_proc);
	init_primitive(&make_integral_fraction, &make_integral_fraction_proc);
	init_primitive(&fraction_reduce, &fraction_reduce_proc);
	init_primitive(&fraction_reduce_two, &fraction_reduce_two_proc);
	init_primitive(&fraction_make_and_reduce, &fraction_make_and_reduce_proc);
	
	init_primitive(&fraction_add, &fraction_add_proc);
	init_primitive(&fraction_subtract, &fraction_subtract_proc);
	init_primitive(&fraction_negate, &fraction_negate_proc);
	init_primitive(&fraction_multiply, &fraction_multiply_proc);
	init_primitive(&fraction_divide, &fraction_divide_proc);
	
	init_primitive(&fraction_quotient_remainder, &fraction_quotient_remainder_proc);
	init_primitive(&fraction_quotient_remainder_two, &fraction_quotient_remainder_two_proc);
	init_primitive(&fraction_quotient_remainder_three, &fraction_quotient_remainder_three_proc);
	
	init_primitive(&fraction_quotient, &fraction_quotient_proc);
	init_primitive(&fraction_quotient_continued, &fraction_quotient_continued_proc);
	
	init_primitive(&fraction_remainder, &fraction_remainder_proc);
	init_primitive(&fraction_remainder_continued, &fraction_remainder_continued_proc);
	
	init_primitive(&fraction_compare, &fraction_compare_proc);
}
