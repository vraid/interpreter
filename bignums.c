#include "bignums.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"
#include "list-util.h"

object* first_or_zero(object* ls) {
	return is_empty_list(ls) ? zero() : list_first(ls);
}

object* make_bignum(object* args, object* cont) {
	object* digits;
	object* sign;
	delist_2(args, &digits, &sign);
	
	object num;
	init_bignum(&num, fixnum_value(sign), digits);
	
	return call_cont(cont, &num);
}

object remove_leading_zeroes_proc;

object* remove_leading_zeroes(object* args, object* cont) {
	object* digits;
	delist_1(args, &digits);
	
	while (!is_empty_list(digits) && fixnum_value(list_first(digits)) == 0) {
		digits = list_rest(digits);
	}
	
	return call_cont(cont, is_empty_list(digits) ? bignum_zero_list() : digits);
}

object remove_leading_zeroes_and_reverse_proc;

object* remove_leading_zeroes_and_reverse(object* args, object* cont) {
	object* digits;
	delist_1(args, &digits);
	
	object reverse_call;
	init_call(&reverse_call, &reverse_list_proc, empty_list(), cont);
	object reverse_cont;
	init_cont(&reverse_cont, &reverse_call);
	
	object remove_zeroes_call;
	init_call(&remove_zeroes_call, &remove_leading_zeroes_proc, args, &reverse_cont);
	
	return perform_call(&remove_zeroes_call);
}

object bignum_add_digits_proc;

object* bignum_add_digits(object* args, object* cont) {
	object* last;
	object* carry;
	object* a;
	object* b;
	delist_4(args, &last, &carry, &a, &b);
	
	if (is_empty_list(a) && is_empty_list(b) && is_zero(carry)) {
		return call_cont(cont, last);
	}
	else {
		object* anum = first_or_zero(a);
		object* bnum = first_or_zero(b);
		long n = fixnum_value(anum) + fixnum_value(bnum) + fixnum_value(carry);
		object* next_carry = zero();
		if (n >= bignum_base) {
			next_carry = one();
			n -= bignum_base;
		}
		object num;
		init_fixnum(&num, n);
		object list_cell;
		init_list_cell(&list_cell, &num, last);
		
		object next_args[4];
		init_list_4(next_args, &list_cell, next_carry, list_rest_or_empty(a), list_rest_or_empty(b));
		object next_call;
		init_call(&next_call, &bignum_add_digits_proc, next_args, cont);
		
		return perform_call(&next_call);
	}
}

object bignum_add_signless_proc;

object* bignum_add_signless(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object reverse_call;
	init_call(&reverse_call, &remove_leading_zeroes_and_reverse_proc, empty_list(), cont);
	object reverse_cont;
	init_cont(&reverse_cont, &reverse_call);
	
	object add_args[4];
	init_list_4(add_args, empty_list(), zero(), a, b);
	object add_call;
	init_call(&add_call, &bignum_add_digits_proc, add_args, &reverse_cont);
	
	return perform_call(&add_call);
}

object bignum_add_with_sign_proc;

object* bignum_add_with_sign(object* args, object* cont) {
	object* a;
	object* b;
	object* sign;
	delist_3(args, &a, &b, &sign);
	
	object make_args[1];
	init_list_1(make_args, sign);
	object make_call;
	init_call(&make_call, &make_bignum_proc, make_args, cont);
	object make_cont;
	init_cont(&make_cont, &make_call);
	
	object add_args[2];
	init_list_2(add_args, a, b);
	object add_call;
	init_call(&add_call, &bignum_add_signless_proc, add_args, &make_cont);
	
	return perform_call(&add_call);
}

object* bignum_add(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	int asign = bignum_sign(a);
	int bsign = bignum_sign(b);
	
	int sign_sum = asign + bsign;
	
	object* a_digits = bignum_digits(a);
	object* b_digits = bignum_digits(b);
	
	if (sign_sum == 0) {
		char a_negative = asign < 0;
		object subtrahend;
		init_bignum(&subtrahend, 1, a_negative ? a_digits : b_digits);
		object minuend;
		init_bignum(&minuend, 1, a_negative ? b_digits : a_digits);
		
		object subtract_args[2];
		init_list_2(subtract_args, &subtrahend, &minuend);
		object subtract_call;
		init_call(&subtract_call, &bignum_subtract_proc, subtract_args, cont);
		
		return perform_call(&subtract_call);
	}
	else {		
		object add_args[3];
		init_list_3(add_args, a_digits, b_digits, sign_sum > 0 ? one() : negative_one());
		object add_call;
		init_call(&add_call, &bignum_add_with_sign_proc, add_args, cont);
		
		return perform_call(&add_call);
	}
}

object bignum_subtract_digits_proc;

object* bignum_subtract_digits(object* args, object* cont) {
	object* last;
	object* carry;
	object* subtrahend;
	object* minuend;
	delist_4(args, &last, &carry, &subtrahend, &minuend);
	
	char subtrahend_empty = is_empty_list(subtrahend);
	
	object* subtrahend_first = subtrahend_empty ? zero() : list_first(subtrahend);
	object* subtrahend_rest = list_rest_or_empty(subtrahend);
	
	char minuend_empty = is_empty_list(minuend);
	
	object* minuend_first = minuend_empty ? zero() : list_first(minuend);
	object* minuend_rest = list_rest_or_empty(minuend);
	
	long n = fixnum_value(minuend_first) - fixnum_value(carry) - fixnum_value(subtrahend_first);
	
	object* next_carry = zero();
	if (n < 0) {
		next_carry = one();
		n += bignum_base;
	}
	
	object num;
	init_fixnum(&num, n);
	
	object cell;
	init_list_cell(&cell, &num, last);
	
	if (is_empty_list(subtrahend)) {
		object append_args[2];
		init_list_2(append_args, minuend_rest, last);
		object append_call;
		init_call(&append_call, &list_append_first_reversed_proc, append_args, cont);
		
		return perform_call(&append_call);
	}
	else {
		object next_args[4];
		init_list_4(next_args, &cell, carry, subtrahend_rest, minuend_rest);
		object next_call;
		init_call(&next_call, &bignum_subtract_digits_proc, next_args, cont);
		
		return perform_call(&next_call);
	}
}

object bignum_subtract_with_sign_proc;

object* bignum_subtract_with_sign(object* args, object* cont) {
	object* subtrahend;
	object* minuend;
	object* sign;
	delist_3(args, &subtrahend, &minuend, &sign);
	
	object make_args[1];
	init_list_1(make_args, sign);
	object make_call;
	init_call(&make_call, &make_bignum_proc, make_args, cont);
	object make_cont;
	init_cont(&make_cont, &make_call);
	
	object reverse_call;
	init_call(&reverse_call, &remove_leading_zeroes_and_reverse_proc, empty_list(), &make_cont);
	object reverse_cont;
	init_cont(&reverse_cont, &reverse_call);
	
	object trim_call;
	init_call(&trim_call, &remove_leading_zeroes_proc, empty_list(), &reverse_cont);
	object trim_cont;
	init_cont(&trim_cont, &trim_call);
	
	object subtract_args[4];
	init_list_4(subtract_args, empty_list(), zero(), subtrahend, minuend);
	object subtract_call;
	init_call(&subtract_call, &bignum_subtract_digits_proc, subtract_args, &trim_cont);
	
	return perform_call(&subtract_call);
}

int signum(long a) {
	if (a > 0) return 1;
	else if (a < 0) return -1;
	else return 0;
}

int compare_bignums(object* a, object* b) {
	int compare = 0;
	
	object* as = bignum_digits(a);
	object* bs = bignum_digits(b);
	
	while (!is_empty_list(as) && !is_empty_list(bs)) {
		int c = signum(fixnum_value(list_first(as)) - fixnum_value(list_first(bs)));
		if (c != 0) {
			compare = c;
		}
		as = list_rest(as);
		bs = list_rest(bs);
	}
	char a_empty = is_empty_list(as);
	char b_empty = is_empty_list(bs);
	if (a_empty && !b_empty) {
		compare = -1;
	}
	else if (a_empty && !b_empty) {
		compare = 1;
	}
	return compare;
}

object* bignum_subtract(object* args, object* cont) {
	object* subtrahend;
	object* minuend;
	delist_2(args, &subtrahend, &minuend);
	
	int subsign = bignum_sign(subtrahend);
	int minsign = bignum_sign(minuend);
	
	int sign_sum = subsign + minsign;
	
	object* subd = bignum_digits(subtrahend);
	object* mind = bignum_digits(minuend);
	
	// addition
	if (sign_sum == 0) {		
		object add_args[3];
		init_list_3(add_args, subd, mind, subsign < 0 ? one() : negative_one());
		object add_call;
		init_call(&add_call, &bignum_add_with_sign_proc, add_args, cont);
		
		return perform_call(&add_call);
	}
	// subtraction
	else {
		int compare = compare_bignums(subtrahend, minuend);
		if (compare == 0) {
			return call_cont(cont, bignum_zero());
		}
		else {
			char reverse = compare > 0;
			object subtract_args[3];
			if (reverse) {
				init_list_3(subtract_args, mind, subd, negative_one());
			}
			else {
				init_list_3(subtract_args, subd, mind, one());
			}
			
			object subtract_call;
			init_call(&subtract_call, &bignum_subtract_with_sign_proc, subtract_args, cont);
			
			return perform_call(&subtract_call);
		}
	}
}

object bignum_multiply_add_proc;

object* bignum_multiply_add(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object b_shifted;
	init_list_cell(&b_shifted, zero(), b);
	
	object add_args[2];
	init_list_2(add_args, a, &b_shifted);
	object add_call;
	init_call(&add_call, &bignum_add_signless_proc, add_args, cont);
	
	return perform_call(&add_call);
}

object bignum_multiply_digit_proc;

object* bignum_multiply_digit(object* args, object* cont) {
	object* last;
	object* carry;
	object* a;
	object* b;
	object* b_rest;
	delist_5(args, &last, &carry, &a, &b, &b_rest);
	
	long product = fixnum_value(a) * fixnum_value(b) + fixnum_value(carry);
	long result_value = product & (bignum_base - 1);
	long carry_value = product >> bignum_base_bits;
	
	object result;
	init_fixnum(&result, result_value);
	object next_carry;
	init_fixnum(&next_carry, carry_value);
	
	object cell;
	init_list_cell(&cell, &result, last);
	
	if (is_empty_list(b_rest)) {
		object carry_cell;
		init_list_cell(&carry_cell, &next_carry, &cell);
		
		object reverse_args[1];
		init_list_1(reverse_args, &carry_cell);
		object reverse_call;
		init_call(&reverse_call, &remove_leading_zeroes_and_reverse_proc, reverse_args, cont);
		
		return perform_call(&reverse_call);
	}
	else {
		object next_args[5];
		init_list_5(next_args, &cell, &next_carry, a, list_first(b_rest), list_rest(b_rest));
		object next_call;
		init_call(&next_call, &bignum_multiply_digit_proc, next_args, cont);
		
		return perform_call(&next_call);
	}
}

object bignum_multiply_digits_proc;

object* bignum_multiply_digits(object* args, object* cont) {
	object* result;
	object* a;
	object* b;
	delist_3(args, &result, &a, &b);
	
	if (is_empty_list(b)) {
		return call_cont(cont, result);
	}
	else {
		object next_args[2];
		init_list_2(next_args, a, list_rest(b));
		object next_call;
		init_call(&next_call, &bignum_multiply_digits_proc, next_args, cont);
		object next_cont;
		init_cont(&next_cont, &next_call);
		
		object add_args[1];
		init_list_1(add_args, result);
		object add_call;
		init_call(&add_call, &bignum_multiply_add_proc, add_args, &next_cont);
		object add_cont;
		init_cont(&add_cont, &add_call);
		
		object multiply_args[5];
		init_list_5(multiply_args, empty_list(), zero(), list_first(b), list_first(a), list_rest(a));
		object multiply_call;
		init_call(&multiply_call, &bignum_multiply_digit_proc, multiply_args, &add_cont);
		
		return perform_call(&multiply_call);
	}
}

object bignum_multiply_reversed_proc;

object* bignum_multiply_reversed(object* args, object* cont) {
	object* reversed;
	object* a;
	delist_2(args, &reversed, &a);
	
	object multiply_args[3];
	init_list_3(multiply_args, bignum_zero_list(), a, reversed);
	object multiply_call;
	init_call(&multiply_call, &bignum_multiply_digits_proc, multiply_args, cont);
	
	return perform_call(&multiply_call);
}

object* bignum_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	int sign = bignum_sign(a) * bignum_sign(b);
	object* a_digits = bignum_digits(a);
	object* b_digits = bignum_digits(b);
	
	object make_args[1];
	init_list_1(make_args, sign_object(sign));
	object make_call;
	init_call(&make_call, &make_bignum_proc, make_args, cont);
	object make_cont;
	init_cont(&make_cont, &make_call);
	
	object multiply_args[1];
	init_list_1(multiply_args, b_digits);
	object multiply_call;
	init_call(&multiply_call, &bignum_multiply_reversed_proc, multiply_args, &make_cont);
	object multiply_cont;
	init_cont(&multiply_cont, &multiply_call);
	
	object reverse_args[1];
	init_list_1(reverse_args, a_digits);
	object reverse_call;
	init_call(&reverse_call, &reverse_list_proc, reverse_args, &multiply_cont);
	
	return perform_call(&reverse_call);
}

object bignum_to_string_proc;

object* bignum_to_string(object* args, object* cont) {
	object* number;
	delist_1(args, &number);
	
	return call_cont(cont, no_object());
}

void init_bignum_procedures(void) {
	init_primitive_procedure(&make_bignum_proc, &make_bignum);
	init_primitive_procedure(&bignum_add_proc, &bignum_add);
	init_primitive_procedure(&bignum_add_signless_proc, &bignum_add_signless);
	init_primitive_procedure(&bignum_add_with_sign_proc, &bignum_add_with_sign);
	init_primitive_procedure(&bignum_add_digits_proc, &bignum_add_digits);
	
	init_primitive_procedure(&remove_leading_zeroes_proc, &remove_leading_zeroes);
	init_primitive_procedure(&remove_leading_zeroes_and_reverse_proc, &remove_leading_zeroes_and_reverse);
	
	init_primitive_procedure(&bignum_subtract_proc, &bignum_subtract);
	init_primitive_procedure(&bignum_subtract_with_sign_proc, &bignum_subtract_with_sign);
	init_primitive_procedure(&bignum_subtract_digits_proc, &bignum_subtract_digits);

	init_primitive_procedure(&bignum_multiply_proc, &bignum_multiply);
	init_primitive_procedure(&bignum_multiply_reversed_proc, &bignum_multiply_reversed);
	init_primitive_procedure(&bignum_multiply_digits_proc, &bignum_multiply_digits);
	init_primitive_procedure(&bignum_multiply_digit_proc, &bignum_multiply_digit);
	init_primitive_procedure(&bignum_multiply_add_proc, &bignum_multiply_add);
	
	init_primitive_procedure(&bignum_to_string_proc, &bignum_to_string);
}
