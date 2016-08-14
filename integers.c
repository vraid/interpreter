#include "integers.h"

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

char integers_have_different_signs(object* a, object* b) {
	return integer_sign(a) != integer_sign(b);
}

object* make_integer(object* args, object* cont) {
	object* digits;
	object* sign;
	delist_2(args, &digits, &sign);
	
	if (digits_have_value(0, digits)) {
		return call_cont(cont, integer_zero());
	}
	else {
		object num;
		init_integer(&num, fixnum_value(sign), digits);
		
		return call_cont(cont, &num);
	}
}

object remove_leading_zeroes_proc;

object* remove_leading_zeroes(object* args, object* cont) {
	object* digits;
	delist_1(args, &digits);
	
	while (!is_empty_list(digits) && fixnum_value(list_first(digits)) == 0) {
		digits = list_rest(digits);
	}
	
	return call_cont(cont, is_empty_list(digits) ? integer_zero_list() : digits);
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

object* integer_add_digits(object* args, object* cont) {
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
		if (n >= integer_base) {
			carry = 1;
			n -= integer_base;
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

object* integer_add_signless(object* args, object* cont) {
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
	init_call(&add_call, &integer_add_digits_proc, add_args, &reverse_cont);
	
	return perform_call(&add_call);
}

object integer_add_with_sign_proc;

object* integer_add_with_sign(object* args, object* cont) {
	object* a;
	object* b;
	object* sign;
	delist_3(args, &a, &b, &sign);
	
	object make_args[1];
	init_list_1(make_args, sign);
	object make_call;
	init_call(&make_call, &make_integer_proc, make_args, cont);
	object make_cont;
	init_cont(&make_cont, &make_call);
	
	object add_args[2];
	init_list_2(add_args, a, b);
	object add_call;
	init_call(&add_call, &integer_add_signless_proc, add_args, &make_cont);
	
	return perform_call(&add_call);
}

object* integer_add(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	int asign = integer_sign(a);
	int bsign = integer_sign(b);
	
	int sign_sum = asign + bsign;
	
	object* a_digits = integer_digits(a);
	object* b_digits = integer_digits(b);
	
	if (sign_sum == 0) {
		char a_negative = asign < 0;
		object subtrahend;
		init_integer(&subtrahend, 1, a_negative ? a_digits : b_digits);
		object minuend;
		init_integer(&minuend, 1, a_negative ? b_digits : a_digits);
		
		object subtract_args[2];
		init_list_2(subtract_args, &subtrahend, &minuend);
		object subtract_call;
		init_call(&subtract_call, &integer_subtract_proc, subtract_args, cont);
		
		return perform_call(&subtract_call);
	}
	else {		
		object add_args[3];
		init_list_3(add_args, a_digits, b_digits, sign_sum > 0 ? one() : negative_one());
		object add_call;
		init_call(&add_call, &integer_add_with_sign_proc, add_args, cont);
		
		return perform_call(&add_call);
	}
}

object integer_subtract_digits_proc;

object* integer_subtract_digits(object* args, object* cont) {
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
			n += integer_base;
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

object integer_subtract_with_sign_proc;

object* integer_subtract_with_sign(object* args, object* cont) {
	object* subtrahend;
	object* minuend;
	object* sign;
	delist_3(args, &subtrahend, &minuend, &sign);
	
	object make_args[1];
	init_list_1(make_args, sign);
	object make_call;
	init_call(&make_call, &make_integer_proc, make_args, cont);
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
	init_call(&subtract_call, &integer_subtract_digits_proc, subtract_args, &trim_cont);
	
	return perform_call(&subtract_call);
}

int signum(long a) {
	if (a > 0) return 1;
	else if (a < 0) return -1;
	else return 0;
}

char digits_have_value(int value, object* digits) {
	return is_empty_list(list_rest(digits)) && (value == fixnum_value(list_first(digits)));
}

char is_one_value_integer(int value, object* a) {
	return digits_have_value(value, integer_digits(a));
}

char is_zero_integer(object* a) {
	return is_one_value_integer(0, a);
}

char is_one_integer(object* a) {
	return is_positive_integer(a) && is_one_value_integer(1, a);
}

char is_positive_integer(object* a) {
	return (integer_sign(a) == 1) && !is_zero_integer(a);
}

char is_negative_integer(object* a) {
	return (integer_sign(a) == -1) && !is_zero_integer(a);
}

int compare_unsigned_integers(object* a, object* b) {
	int compare = 0;
	
	object* as = integer_digits(a);
	object* bs = integer_digits(b);
	
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

int compare_signed_integers(object* a, object* b) {
	if (is_zero_integer(a) && is_zero_integer(b)) {
		return 0;
	}
	else {
		int a_sign = integer_sign(a);
		int b_sign = integer_sign(b);
		
		if (a_sign == b_sign) {
			return a_sign * compare_unsigned_integers(a, b);
		}
		else {
			return (a_sign == 1) ? 1 : -1;
		}
	}
}

object* integer_subtract(object* args, object* cont) {
	object* subtrahend;
	object* minuend;
	delist_2(args, &subtrahend, &minuend);
	
	int subsign = integer_sign(subtrahend);
	int minsign = integer_sign(minuend);
	
	int sign_sum = subsign + minsign;
	
	object* subd = integer_digits(subtrahend);
	object* mind = integer_digits(minuend);
	
	// addition
	if (sign_sum == 0) {		
		object add_args[3];
		init_list_3(add_args, subd, mind, subsign < 0 ? one() : negative_one());
		object add_call;
		init_call(&add_call, &integer_add_with_sign_proc, add_args, cont);
		
		return perform_call(&add_call);
	}
	// subtraction
	else {
		int compare = compare_unsigned_integers(subtrahend, minuend);
		if (compare == 0) {
			return call_cont(cont, integer_zero());
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
			init_call(&subtract_call, &integer_subtract_with_sign_proc, subtract_args, cont);
			
			return perform_call(&subtract_call);
		}
	}
}

object integer_multiply_add_proc;

object* integer_multiply_add(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object b_shifted;
	init_list_cell(&b_shifted, zero(), b);
	
	object add_args[2];
	init_list_2(add_args, a, &b_shifted);
	object add_call;
	init_call(&add_call, &integer_add_signless_proc, add_args, cont);
	
	return perform_call(&add_call);
}

object* integer_multiply_digit(object* args, object* cont) {
	object* a_num;
	object* b;
	delist_2(args, &a_num, &b);
	
	object* b_num;
	long carry = 0;
	
	object* digits = empty_list();
	
	while (!(is_empty_list(b) && (carry == 0))) {
		b_num = first_or_zero(b);
		
		long product = fixnum_value(a_num) * fixnum_value(b_num) + carry;
		long result_value = product & (integer_base - 1);
		carry = product >> integer_base_bits;
		
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

object integer_multiply_digits_proc;

object* integer_multiply_digits(object* args, object* cont) {
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
		init_call(&next_call, &integer_multiply_digits_proc, next_args, cont);
		object next_cont;
		init_cont(&next_cont, &next_call);
		
		object add_args[1];
		init_list_1(add_args, result);
		object add_call;
		init_call(&add_call, &integer_multiply_add_proc, add_args, &next_cont);
		object add_cont;
		init_cont(&add_cont, &add_call);
		
		object multiply_args[2];
		init_list_2(multiply_args, list_first(b), a);
		object multiply_call;
		init_call(&multiply_call, &integer_multiply_digit_proc, multiply_args, &add_cont);
		
		return perform_call(&multiply_call);
	}
}

object integer_multiply_reversed_proc;

object* integer_multiply_reversed(object* args, object* cont) {
	object* reversed;
	object* a;
	delist_2(args, &reversed, &a);
	
	object multiply_args[3];
	init_list_3(multiply_args, integer_zero_list(), a, reversed);
	object multiply_call;
	init_call(&multiply_call, &integer_multiply_digits_proc, multiply_args, cont);
	
	return perform_call(&multiply_call);
}

object* integer_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	int sign = integer_sign(a) * integer_sign(b);
	object* a_digits = integer_digits(a);
	object* b_digits = integer_digits(b);
	
	object make_args[1];
	init_list_1(make_args, sign_object(sign));
	object make_call;
	init_call(&make_call, &make_integer_proc, make_args, cont);
	object make_cont;
	init_cont(&make_cont, &make_call);
	
	object multiply_args[1];
	init_list_1(multiply_args, b_digits);
	object multiply_call;
	init_call(&multiply_call, &integer_multiply_reversed_proc, multiply_args, &make_cont);
	object multiply_cont;
	init_cont(&multiply_cont, &multiply_call);
	
	object reverse_args[1];
	init_list_1(reverse_args, a_digits);
	object reverse_call;
	init_call(&reverse_call, &reverse_list_proc, reverse_args, &multiply_cont);
	
	return perform_call(&reverse_call);
}

object integer_divide_one_proc;

object* integer_divide_one(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	object* divisor_digits = integer_digits(divisor);
	long divisor_first = 0; 
	long power = 1;
	
	while (!is_empty_list(divisor_digits)) {
		divisor_first = fixnum_value(list_first(divisor_digits));
		divisor_digits = list_rest(divisor_digits);
		power--;
	}
	
	object* dividend_digits = integer_digits(dividend);
	long dividend_first = fixnum_value(list_first(dividend_digits));
	long dividend_second = 0;

	dividend_digits = list_rest(dividend_digits);

	while (!is_empty_list(dividend_digits)) {
		dividend_second = dividend_first;
		dividend_first = fixnum_value(list_first(dividend_digits));
		dividend_digits = list_rest(dividend_digits);
		power++;
	}
	
	if (dividend_first < divisor_first) {
		dividend_first = dividend_second + dividend_first * integer_base;
		power--;
	}
	
	long result = dividend_first / divisor_first;
	
	object num;
	init_fixnum(&num, result);
	object first_cell;
	init_list_cell(&first_cell, &num, empty_list());
	object* digits = &first_cell;
	int i;
	for (i = power; i > 0; i--) {
		object* next_cell = alloca(sizeof(object));
		init_list_cell(next_cell, zero(), digits);
		digits = next_cell;
	}
	
	object bign;
	init_positive_integer(&bign, digits);
	
	return call_cont(cont, &bign);
}

object integer_perform_division_proc;
object integer_adjust_dividend_proc;

object* integer_adjust_dividend(object* args, object* cont) {
	object* quotient;
	object* divisor;
	object* dividend;
	delist_3(args, &quotient, &divisor, &dividend);
	
	object continue_call;
	init_call(&continue_call, &integer_perform_division_proc, args, cont);
	object continue_cont;
	init_cont(&continue_cont, &continue_call);
	
	object subtract_args[1];
	init_list_1(subtract_args, dividend);
	object subtract_call;
	init_call(&subtract_call, &integer_subtract_proc, subtract_args, &continue_cont);
	object subtract_cont;
	init_cont(&subtract_cont, &subtract_call);
	
	object multiply_args[2];
	init_list_2(multiply_args, quotient, divisor);
	object multiply_call;
	init_call(&multiply_call, &integer_multiply_proc, multiply_args, &subtract_cont);
	
	return perform_call(&multiply_call);
}

object* integer_perform_division(object* args, object* cont) {
	object* difference;
	object* quotient;
	object* divisor;
	object* dividend;
	delist_4(args, &difference, &quotient, &divisor, &dividend);
	
	char difference_positive = integer_sign(difference) == 1;
	
	int compare = compare_unsigned_integers(divisor, difference);
	
	if (is_zero_integer(difference) || (compare == 1 && difference_positive)) {
		object ls[2];
		init_list_2(ls, quotient, difference);
		
		return call_cont(cont, ls);
	}
	else {
		object* adjust_quotient_proc = difference_positive ? &integer_add_proc : &integer_subtract_proc;
		
		object difference_args[2];
		init_list_2(difference_args, divisor, dividend);
		object difference_call;
		init_call(&difference_call, &integer_adjust_dividend_proc, difference_args, cont);
		object difference_cont;
		init_cont(&difference_cont, &difference_call);
		
		object quotient_args[1];
		init_list_1(quotient_args, quotient);
		object quotient_call;
		init_call(&quotient_call, adjust_quotient_proc, quotient_args, &difference_cont);
		object quotient_cont;
		init_cont(&quotient_cont, &quotient_call);
		
		object divide_args[2];
		init_list_2(divide_args, divisor, difference);
		object divide_call;
		init_call(&divide_call, &integer_divide_one_proc, divide_args, &quotient_cont);
		
		return perform_call(&divide_call);
	}
}

object integer_make_division_result_proc;

object* integer_make_division_result(object* args, object* cont) {
	object* quot_rem;
	object* quotient_sign;
	object* remainder_sign;
	delist_3(args, &quot_rem, &quotient_sign, &remainder_sign);
	
	object* quot;
	object* rem;
	delist_2(quot_rem, &quot, &rem);
	
	object quotient;
	init_integer(&quotient, fixnum_value(quotient_sign), integer_digits(quot));
	object remainder;
	init_integer(&remainder, fixnum_value(remainder_sign), integer_digits(rem));
	
	object ls[2];
	init_list_2(ls, &quotient, &remainder);
	
	return call_cont(cont, ls);
}

object* integer_divide(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	int divisor_sign = integer_sign(divisor);
	int dividend_sign = integer_sign(dividend);
	
	int quotient_sign = divisor_sign * dividend_sign;
	int remainder_sign = dividend_sign;
	
	object positive_divisor;
	init_positive_integer(&positive_divisor, integer_digits(divisor));
	
	object positive_dividend;
	init_positive_integer(&positive_dividend, integer_digits(dividend));
	
	object result_args[2];
	init_list_2(result_args, sign_object(quotient_sign), sign_object(remainder_sign));
	object result_call;
	init_call(&result_call, &integer_make_division_result_proc, result_args, cont);
	object result_cont;
	init_cont(&result_cont, &result_call);
	
	object divide_args[4];
	init_list_4(divide_args, &positive_dividend, integer_zero(), &positive_divisor, &positive_dividend);
	object divide_call;
	init_call(&divide_call, &integer_perform_division_proc, divide_args, &result_cont);
	
	return perform_call(&divide_call);
}

object integer_gcd_step_proc;

object* integer_gcd_step(object* args, object* cont) {
	object* quot_rem;
	object* smaller;
	object* larger;
	delist_3(args, &quot_rem, &smaller, &larger);
	
	object* quotient;
	object* remainder;
	delist_2(quot_rem, &quotient, &remainder);
	
	if (is_zero_integer(remainder)) {
		return call_cont(cont, smaller);
	}
	else {
		object gcd_args[2];
		init_list_2(gcd_args, remainder, smaller);
		object gcd_call;
		init_call(&gcd_call, &integer_gcd_step_proc, gcd_args, cont);
		object gcd_cont;
		init_cont(&gcd_cont, &gcd_call);
		
		object divide_args[2];
		init_list_2(divide_args, remainder, smaller);
		object divide_call;
		init_call(&divide_call, &integer_divide_proc, divide_args, &gcd_cont);
		
		return perform_call(&divide_call);
	}
}


object integer_quotient_continued_proc;

object* integer_quotient_continued(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	return call_cont(cont, list_first(ls));
}

object* integer_quotient(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	object next_call;
	init_call(&next_call, &integer_quotient_continued_proc, empty_list(), cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	
	object call;
	init_call(&call, &integer_divide_proc, args, &next_cont);
	
	return perform_call(&call);
}

object integer_remainder_continued_proc;

object* integer_remainder_continued(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	return call_cont(cont, list_ref(1, ls));
}

object* integer_remainder(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	object next_call;
	init_call(&next_call, &integer_remainder_continued_proc, empty_list(), cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	
	object call;
	init_call(&call, &integer_divide_proc, args, &next_cont);
	
	return perform_call(&call);
}

object* integer_greatest_common_divisor(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object a_pos;
	init_positive_integer(&a_pos, integer_digits(a));
	object b_pos;
	init_positive_integer(&b_pos, integer_digits(b));
	
	int compare = compare_unsigned_integers(&a_pos, &b_pos);
	
	if (compare == 0) {
		return call_cont(cont, &a_pos);
	}
	else {
		char a_smaller = (compare == -1);
		object* smaller = a_smaller ? &a_pos : &b_pos;
		object* larger = a_smaller ? &b_pos : &a_pos;
		
		object quot_rem[2];
		init_list_2(quot_rem, integer_zero(), smaller);
		object gcd_args[3];
		init_list_3(gcd_args, quot_rem, larger, integer_zero());
		object gcd_call;
		init_call(&gcd_call, &integer_gcd_step_proc, gcd_args, cont);
		
		return perform_call(&gcd_call);
	}
}

object* integer_subtract_one(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	object subtract_args[2];
	init_list_2(subtract_args, integer_one(), a);
	object subtract_call;
	init_call(&subtract_call, &integer_subtract_proc, subtract_args, cont);
	
	return perform_call(&subtract_call);
}

object integer_digits_to_new_base_proc;

object* integer_digits_to_new_base(object* args, object* cont) {
	object* quot_rem;
	object* last;
	object* base;
	delist_3(args, &quot_rem, &last, &base);
	
	object* quotient;
	object* remainder;
	delist_2(quot_rem, &quotient, &remainder);
	
	// assumes remainder only has one digit, which holds true for conversion to bases <= integer_base
	object* num = list_first(integer_digits(remainder));
	object cell;
	init_list_cell(&cell, num, last);
	
	if (is_zero_integer(quotient)) {
		return call_cont(cont, &cell);
	}
	else {
		object next_args[2];
		init_list_2(next_args, &cell, base);
		object next_call;
		init_call(&next_call, &integer_digits_to_new_base_proc, next_args, cont);
		object next_cont;
		init_cont(&next_cont, &next_call);
		
		object divide_args[2];
		init_list_2(divide_args, base, quotient);
		object divide_call;
		init_call(&divide_call, &integer_divide_proc, divide_args, &next_cont);
		
		return perform_call(&divide_call);
	}
}

object integer_to_new_base_proc;

// returns a list of digits in the new base, from largest to smallest (opposite to order in integer objects)

object* integer_to_new_base(object* args, object* cont) {
	object* num;
	object* base;
	delist_2(args, &num, &base);
	
	object next_args[2];
	init_list_2(next_args, empty_list(), base);
	object next_call;
	init_call(&next_call, &integer_digits_to_new_base_proc, next_args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	
	object divide_args[2];
	init_list_2(divide_args, base, num);
	object divide_call;
	init_call(&divide_call, &integer_divide_proc, divide_args, &next_cont);
	
	return perform_call(&divide_call);
}

object* integer_to_decimal(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	object positive_num;
	init_positive_integer(&positive_num, integer_digits(num));
	
	object call_args[2];
	init_list_2(call_args, &positive_num, integer_ten());
	object call;
	init_call(&call, &integer_to_new_base_proc, call_args, cont);
	
	return perform_call(&call);
}

object decimal_to_string_proc;

object* decimal_to_string(object* args, object* cont) {
	object* digits;
	object* sign;
	delist_2(args, &digits, &sign);
	
	int length = list_length(digits);
	char negative = fixnum_value(sign) == -1;
	
	int string_length = length + negative;
	
	char* str = alloca(sizeof(char) * (string_length + 1));
	
	if (negative) {
		str[0] = '-';
	}
	
	int i;
	for (i = !negative; i < string_length; i++) {
		str[i] = fixnum_value(list_first(digits)) - '0';
		digits = list_rest(digits);
	}
	
	str[string_length] = 0;
	
	object string;
	init_string(&string, str);
	
	return call_cont(cont, &string);
}

object* integer_to_string(object* args, object* cont) {
	object* number;
	delist_1(args, &number);
	
	object* sign = sign_object(integer_sign(number));
	if (is_zero_integer(number)) {
		sign = sign_object(1);
	}
	
	object string_args[1];
	init_list_1(string_args, sign);
	object string_call;
	init_call(&string_call, &decimal_to_string_proc, string_args, cont);
	object string_cont;
	init_cont(&string_cont, &string_call);
	
	object decimal_call;
	init_call(&decimal_call, &integer_to_new_base_proc, args, &string_cont);
	
	return perform_call(&decimal_call);
}

void init_integer_procedures(void) {
	init_primitive(&make_integer, &make_integer_proc);
	init_primitive(&integer_add, &integer_add_proc);
	init_primitive(&integer_add_signless, &integer_add_signless_proc);
	init_primitive(&integer_add_with_sign, &integer_add_with_sign_proc);
	init_primitive(&integer_add_digits, &integer_add_digits_proc);
	
	init_primitive(&remove_leading_zeroes, &remove_leading_zeroes_proc);
	init_primitive(&remove_leading_zeroes_and_reverse, &remove_leading_zeroes_and_reverse_proc);
	
	init_primitive(&integer_subtract, &integer_subtract_proc);
	init_primitive(&integer_subtract_with_sign, &integer_subtract_with_sign_proc);
	init_primitive(&integer_subtract_digits, &integer_subtract_digits_proc);
	init_primitive(&integer_subtract_one, &integer_subtract_one_proc);

	init_primitive(&integer_multiply, &integer_multiply_proc);
	init_primitive(&integer_multiply_reversed, &integer_multiply_reversed_proc);
	init_primitive(&integer_multiply_digits, &integer_multiply_digits_proc);
	init_primitive(&integer_multiply_digit, &integer_multiply_digit_proc);
	init_primitive(&integer_multiply_add, &integer_multiply_add_proc);
	
	init_primitive(&integer_divide, &integer_divide_proc);
	init_primitive(&integer_make_division_result, &integer_make_division_result_proc);
	init_primitive(&integer_divide_one, &integer_divide_one_proc);
	init_primitive(&integer_perform_division, &integer_perform_division_proc);
	init_primitive(&integer_adjust_dividend, &integer_adjust_dividend_proc);
	
	init_primitive(&integer_quotient, &integer_quotient_proc);
	init_primitive(&integer_quotient_continued, &integer_quotient_continued_proc);
	init_primitive(&integer_remainder, &integer_remainder_proc);
	init_primitive(&integer_remainder_continued, &integer_remainder_continued_proc);
	
	init_primitive(&integer_greatest_common_divisor, &integer_greatest_common_divisor_proc);
	init_primitive(&integer_gcd_step, &integer_gcd_step_proc);
	
	init_primitive(&integer_to_new_base, &integer_to_new_base_proc);
	init_primitive(&integer_digits_to_new_base, &integer_digits_to_new_base_proc);
	init_primitive(&integer_to_decimal, &integer_to_decimal_proc);
	init_primitive(&decimal_to_string, &decimal_to_string_proc);
	init_primitive(&integer_to_string, &integer_to_string_proc);
}
