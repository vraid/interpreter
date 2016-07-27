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
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* digits = empty_list();
	long carry = 0;
	
	while (!(is_empty_list(a) && is_empty_list(b) && (carry == 0))) {
		object* anum = first_or_zero(a);
		object* bnum = first_or_zero(b);
		
		long n = fixnum_value(anum) + fixnum_value(bnum) + carry;
		carry = 0;
		if (n >= bignum_base) {
			carry = 1;
			n -= bignum_base;
		}
		
		object* next_digit = alloca(sizeof(object));
		init_fixnum(next_digit, n);
		
		object* next_cell = alloca(sizeof(object));
		init_list_cell(next_cell, next_digit, digits);
		
		digits = next_cell;
		
		a = list_rest_or_empty(a);
		b = list_rest_or_empty(b);
	}
	return call_cont(cont, digits);
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
	
	object add_args[2];
	init_list_2(add_args, a, b);
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
	object* subtrahend;
	object* minuend;
	delist_2(args, &subtrahend, &minuend);
	
	object* digits = empty_list();
	long carry = 0;
	
	while (!is_empty_list(minuend)) {
		object* minuend_first = first_or_zero(minuend);
		object* subtrahend_first = first_or_zero(subtrahend);
		
		long n = fixnum_value(minuend_first) - fixnum_value(subtrahend_first) - carry;
		carry = 0;
		if (n < 0) {
			carry = 1;
			n += bignum_base;
		}
		
		object* num = alloca(sizeof(object));
		init_fixnum(num, n);
		
		object* next = alloca(sizeof(object));
		init_list_cell(next, num, digits);

		digits = next;
		
		minuend = list_rest_or_empty(minuend);
		subtrahend = list_rest_or_empty(subtrahend);
	}
	
	return call_cont(cont, digits);
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
	
	object subtract_args[2];
	init_list_2(subtract_args, subtrahend, minuend);
	object subtract_call;
	init_call(&subtract_call, &bignum_subtract_digits_proc, subtract_args, &trim_cont);
	
	return perform_call(&subtract_call);
}

int signum(long a) {
	if (a > 0) return 1;
	else if (a < 0) return -1;
	else return 0;
}

char is_zero_bignum(object* a) {
	object* digits = bignum_digits(a);
	return is_empty_list(list_rest(digits)) && (0 == fixnum_value(list_first(digits)));
}

int compare_unsigned_bignums(object* a, object* b) {
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
	else if (b_empty && !a_empty) {
		compare = 1;
	}
	return compare;
}

int compare_signed_bignums(object* a, object* b) {
	if (is_zero_bignum(a) && is_zero_bignum(b)) {
		return 0;
	}
	else {
		int a_sign = bignum_sign(a);
		int b_sign = bignum_sign(b);
		
		if (a_sign == b_sign) {
			return a_sign * compare_unsigned_bignums(a, b);
		}
		else {
			return (a_sign == 1) ? 1 : -1;
		}
	}
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
		int compare = compare_unsigned_bignums(subtrahend, minuend);
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
	object* a_num;
	object* b;
	delist_2(args, &a_num, &b);
	
	object* b_num;
	long carry = 0;
	
	object* digits = empty_list();
	
	while (!(is_empty_list(b) && (carry == 0))) {
		b_num = first_or_zero(b);
		
		long product = fixnum_value(a_num) * fixnum_value(b_num) + carry;
		long result_value = product & (bignum_base - 1);
		carry = product >> bignum_base_bits;
		
		object* num = alloca(sizeof(object));
		init_fixnum(num, result_value);
		
		object* cell = alloca(sizeof(object));
		init_list_cell(cell, num, digits);

		digits = cell;
		
		b = list_rest_or_empty(b);
	}
		
	object reverse_args[1];
	init_list_1(reverse_args, digits);
	object reverse_call;
	init_call(&reverse_call, &remove_leading_zeroes_and_reverse_proc, reverse_args, cont);
	
	return perform_call(&reverse_call);
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
		
		object multiply_args[2];
		init_list_2(multiply_args, list_first(b), a);
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
