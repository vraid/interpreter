#include "integers.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"
#include "list-util.h"
#include "workspace.h"
#include "print.h"

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
		object* num = alloc_integer(fixnum_value(sign), digits);
		return call_cont(cont, num);
	}
}

int signum(fixnum_type a) {
	if (a > 0) return 1;
	else if (a < 0) return -1;
	else return 0;
}

typedef struct {
	long length;
	fixnum_type* digits;
} workspace_digits;

void workspace_allocate_digits(workspace_digits* a, long length) {
	a->length = length;
	fixnum_type* digits = (fixnum_type*)workspace_allocate(sizeof(fixnum_type) * length);
	long i;
	for (i = 0; i < length; i++) {
		digits[i] = 0;
	}
	a->digits = digits;
}

void workspace_raise_digits(workspace_digits a, long power) {
	long i;
	for (i = a.length-1; i >= 0; i--) {
		a.digits[i] = (i < power) ? 0 : a.digits[i-power];
	}
}

long workspace_highest_digit_index(workspace_digits a) {
	long i;
	for (i = a.length - 1; i >= 0; i--) {
		if (a.digits[i] > 0) {
			return i;
		}
	}
	return 0;
}

int workspace_compare_digits(workspace_digits a, workspace_digits b) {
	long a_first = workspace_highest_digit_index(a);
	long b_first = workspace_highest_digit_index(b);
	
	if (a_first != b_first) {
		return signum(a_first - b_first);
	}
	else {
		long i;
		for (i = a_first; i >= 0; i--) {
			fixnum_type diff = a.digits[i] - b.digits[i];
			if (diff != 0) {
				return signum(diff);
			}
		}
	}
	return 0;
}

void workspace_zero_digits(workspace_digits a) {
	long i;
	for (i = 0; i < a.length; i++) {
		a.digits[i] = 0;
	}
}

void print_workspace_digits(workspace_digits a, char endline) {
	long last = workspace_highest_digit_index(a);
	long i;
	for (i = 0; i <= last; i++) {
		printf("%lld, ", a.digits[i]);
	}
	if (endline) printf("\n");
}

char workspace_digits_are_zero(workspace_digits a) {
	return (workspace_highest_digit_index(a) == 0) && (a.digits[0] == 0);
}

void workspace_list_to_array(workspace_digits target, object* list) {
	long i = 0;
	while (!is_empty_list(list)) {
		target.digits[i] = fixnum_value(list_first(list));
		list = list_rest(list);
		i++;
	}
}

// destructive addition

void workspace_destructive_addition(workspace_digits target, workspace_digits addend) {
	fixnum_type carry = 0;
	long i = 0;
	while ((i < addend.length) || (carry > 0)) {
		fixnum_type total = carry + target.digits[i];
		if (i < addend.length) total += addend.digits[i];

		carry = total >= integer_base ? 1 : 0;
		total -= carry * integer_base;

		target.digits[i] = total;
		i++;
	} 
}

void workspace_subtraction(workspace_digits target, workspace_digits minuend, workspace_digits subtrahend) {
	fixnum_type carry = 0;
	long i = 0;
	
	while ((i < minuend.length) || (carry > 0)) {
		fixnum_type min = minuend.digits[i];
		fixnum_type sub = i < subtrahend.length ? subtrahend.digits[i] : 0; 
		
		fixnum_type n = min - sub - carry;
		carry = 0;
		if (n < 0) {
			carry = 1;
			n += integer_base;
		}
		
		target.digits[i] = n;
		i++;
	}
}

// subtrahend must be <= target. to subtract a larger number, put arguments in reverse order and reverse the sign
void workspace_destructive_subtraction(workspace_digits target, workspace_digits subtrahend) {
	fixnum_type carry = 0;
	long i = 0;
	
	while ((i < subtrahend.length) || (carry > 0)) {
		fixnum_type min = target.digits[i];
		fixnum_type sub = i < subtrahend.length ? subtrahend.digits[i] : 0; 
		
		fixnum_type n = min - sub - carry;
		carry = 0;
		if (n < 0) {
			carry = 1;
			n += integer_base;
		}
		
		target.digits[i] = n;
		i++;
	}
}

void workspace_scalar_multiplication(workspace_digits target, fixnum_type scalar, workspace_digits a) {
	fixnum_type carry = 0;
	long i = 0;
	while ((i < a.length) || (carry > 0)) {
		fixnum_type product = (i >= a.length) ? 0 : scalar * a.digits[i];
		product += carry;
		
		fixnum_type result_value = product & (integer_base - 1);
		carry = product >> integer_base_bits;
		
		target.digits[i] = result_value;
		i++;
	}
}

void workspace_multiplication(workspace_digits target,
                              workspace_digits partial_space,
                              workspace_digits a,
                              workspace_digits b) {
	long i;
	for (i = b.length - 1; i >= 0; i--) {
		fixnum_type b_digit = b.digits[i];
		workspace_zero_digits(partial_space);
		workspace_scalar_multiplication(partial_space, b_digit, a);
		workspace_raise_digits(target, 1);
		workspace_destructive_addition(target, partial_space);
	}
}

char digits_have_value(fixnum_type value, object* digits) {
	return is_empty_list(list_rest(digits)) && (value == fixnum_value(list_first(digits)));
}

char is_one_value_integer(fixnum_type value, object* a) {
	return digits_have_value(value, integer_digits(a));
}

char digits_are_zero(object* ls) {
	return digits_have_value(0, ls);
}

char integer_is_zero(object* a) {
	return integer_sign(a) == 0;
}

char integer_is_one(object* a) {
	return integer_is_positive(a) && is_one_value_integer(1, a);
}

char integer_is_positive(object* a) {
	check_type(type_integer, a);
	return integer_sign(a) == 1;
}

char integer_is_negative(object* a) {
	check_type(type_integer, a);
	return integer_sign(a) == -1;
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
	if (a_empty && b_empty) {
		return compare;
	}
	else {
		return a_empty ? -1 : 1;
	}
}

int compare_signed_integers(object* a, object* b) {
	int a_sign = integer_sign(a);
	int b_sign = integer_sign(b);
	
	if (a_sign == b_sign) {
		return a_sign * compare_unsigned_integers(a, b);
	}
	else {
		return signum(a_sign - b_sign);
	}
}

object* integer_compare(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (integers_have_different_signs(a, b)) {
		return call_cont(cont, (integer_sign(a) == 1) ? one() : negative_one());
	}
	else {
		char c = compare_unsigned_integers(a, b);
		return call_cont(cont, sign_object(c * integer_sign(a)));
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
	
	object* reverse_call = alloc_call(&reverse_list_proc, empty_list(), cont);
	object* remove_zeroes_call = alloc_call(&remove_leading_zeroes_proc, args, alloc_cont(reverse_call));
	return perform_call(remove_zeroes_call);
}

object* integer_add_digits(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* digits = empty_list();
	fixnum_type carry = 0;
	
	while (!(is_empty_list(a) && is_empty_list(b) && (carry == 0))) {
		object* anum = first_or_zero(a);
		object* bnum = first_or_zero(b);
		
		fixnum_type n = fixnum_value(anum) + fixnum_value(bnum) + carry;
		carry = 0;
		if (n >= integer_base) {
			carry = 1;
			n -= integer_base;
		}
		
		object* next_digit = alloc_fixnum(n);
		object* next_cell = alloc_list_cell(next_digit, digits);
		
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
	
	object* reverse_call = alloc_call(&remove_leading_zeroes_and_reverse_proc, empty_list(), cont);
	object* add_args = alloc_list_2(a, b);
	object* add_call = alloc_call(&integer_add_digits_proc, add_args, alloc_cont(reverse_call));
	
	return perform_call(add_call);
}

object integer_add_with_sign_proc;

object* integer_add_with_sign(object* args, object* cont) {
	object* a;
	object* b;
	object* sign;
	delist_3(args, &a, &b, &sign);
	
	object* make_args = alloc_list_1(sign);
	object* make_call = alloc_call(&make_integer_proc, make_args, cont);
	
	object* add_args = alloc_list_2(a, b);
	object* add_call = alloc_call(&integer_add_signless_proc, add_args, alloc_cont(make_call));
	
	return perform_call(add_call);
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
		object* subtrahend = alloc_integer(1, a_negative ? a_digits : b_digits);
		object* minuend = alloc_integer(1, a_negative ? b_digits : a_digits);
		
		object* subtract_args = alloc_list_2(subtrahend, minuend);
		object* subtract_call = alloc_call(&integer_subtract_proc, subtract_args, cont);
		
		return perform_call(subtract_call);
	}
	else {		
		object* add_args = alloc_list_3(a_digits, b_digits, sign_sum > 0 ? one() : negative_one());
		object* add_call = alloc_call(&integer_add_with_sign_proc, add_args, cont);
		
		return perform_call(add_call);
	}
}

object integer_subtract_digits_proc;

object* integer_subtract_digits(object* args, object* cont) {
	object* subtrahend;
	object* minuend;
	delist_2(args, &subtrahend, &minuend);
	
	object* digits = empty_list();
	fixnum_type carry = 0;
	
	while (!is_empty_list(minuend)) {
		object* minuend_first = first_or_zero(minuend);
		object* subtrahend_first = first_or_zero(subtrahend);
		
		fixnum_type n = fixnum_value(minuend_first) - fixnum_value(subtrahend_first) - carry;
		carry = 0;
		if (n < 0) {
			carry = 1;
			n += integer_base;
		}
		
		object* num = alloc_fixnum(n);
		object* next = alloc_list_cell(num, digits);

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
	
	object* make_args = alloc_list_1(sign);
	object* make_call = alloc_call(&make_integer_proc, make_args, cont);
	
	object* reverse_call = alloc_call(&remove_leading_zeroes_and_reverse_proc, empty_list(), alloc_cont(make_call));
	object* trim_call = alloc_call(&remove_leading_zeroes_proc, empty_list(), alloc_cont(reverse_call));
	
	object* subtract_args = alloc_list_2(subtrahend, minuend);
	object* subtract_call = alloc_call(&integer_subtract_digits_proc, subtract_args, alloc_cont(trim_call));
	
	return perform_call(subtract_call);
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
	if (abs(sign_sum) <= 1) {		
		object* add_args = alloc_list_3(subd, mind, subsign < 0 ? one() : negative_one());
		object* add_call = alloc_call(&integer_add_with_sign_proc, add_args, cont);
		
		return perform_call(add_call);
	}
	// subtraction
	else {
		int compare = compare_unsigned_integers(subtrahend, minuend);
		if (compare == 0) {
			return call_cont(cont, integer_zero());
		}
		else {
			char reverse = compare > 0;
			object* subtract_args = reverse ? alloc_list_3(mind, subd, negative_one()) : alloc_list_3(subd, mind, one());			
			object* subtract_call = alloc_call(&integer_subtract_with_sign_proc, subtract_args, cont);
			
			return perform_call(subtract_call);
		}
	}
}

object* integer_negate(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	object* num = alloc_negated_integer(a);
	return call_cont(cont, num);
}

object* integer_multiply_digits(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	long a_length = list_length(a);
	long b_length = list_length(b);
	
	long result_length = a_length + b_length;
	long temporary_length = result_length;
	long combined_count = result_length + a_length + b_length + temporary_length;
	
	reset_workspace(sizeof(fixnum_type) * combined_count);
	
	workspace_digits result;
	workspace_allocate_digits(&result, result_length);
	
	workspace_digits a_digits;
	workspace_allocate_digits(&a_digits, a_length);
	
	workspace_digits b_digits;
	workspace_allocate_digits(&b_digits, b_length);

	workspace_digits temporary;
	workspace_allocate_digits(&temporary, temporary_length);
	
	workspace_list_to_array(a_digits, a);
	workspace_list_to_array(b_digits, b);
	
	workspace_multiplication(result, temporary, a_digits, b_digits);
	
	long last = workspace_highest_digit_index(result);
	object* ls = empty_list();
	long i;
	for (i = last; i >= 0; i--) {
		object* num = alloc_fixnum(result.digits[i]);
		object* cell = alloc_list_cell(num, ls);
		ls = cell;
	}
	
	return call_cont(cont, ls);
}

object* integer_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	int sign = integer_sign(a) * integer_sign(b);
	object* a_digits = integer_digits(a);
	object* b_digits = integer_digits(b);
	
	object* make_args = alloc_list_1(sign_object(sign));
	object* make_call = alloc_call(&make_integer_proc, make_args, cont);
	
	object* multiply_args = alloc_list_2(a_digits, b_digits);
	object* multiply_call = alloc_call(&integer_multiply_digits_proc, multiply_args, alloc_cont(make_call));
	
	return perform_call(multiply_call);
}

object integer_perform_division_proc;

object* integer_perform_division(object* args, object* cont) {
	object* divisor_num;
	object* dividend_num;
	delist_2(args, &divisor_num, &dividend_num);
	
	if (integer_is_one(divisor_num)) {
		object* ls = alloc_list_2(dividend_num, integer_zero());
		return call_cont(cont, ls);
	}
	
	long divisor_length = list_length(integer_digits(divisor_num));
	long dividend_length = list_length(integer_digits(dividend_num));
	long quotient_length = dividend_length;
	long temporary_length = dividend_length;
	long remainder_length = dividend_length;
	long intermediate_quotient_length = quotient_length;
	long intermediate_result_length = dividend_length + 1;
	long combined_length = divisor_length + dividend_length + quotient_length + temporary_length + remainder_length +
	                       intermediate_quotient_length + intermediate_result_length;
	
	reset_workspace(sizeof(fixnum_type) * combined_length);
	
	workspace_digits divisor;
	workspace_allocate_digits(&divisor, divisor_length);
	
	workspace_digits dividend;
	workspace_allocate_digits(&dividend, dividend_length);
	
	workspace_digits quotient;
	workspace_allocate_digits(&quotient, quotient_length);
	
	workspace_digits temporary;
	workspace_allocate_digits(&temporary, temporary_length);
	
	workspace_digits remainder;
	workspace_allocate_digits(&remainder, remainder_length);
	
	workspace_digits intermediate_quotient;
	workspace_allocate_digits(&intermediate_quotient, intermediate_quotient_length);
	
	workspace_digits intermediate_result;
	workspace_allocate_digits(&intermediate_result, intermediate_result_length);
	
	workspace_list_to_array(divisor, integer_digits(divisor_num));
	workspace_list_to_array(dividend, integer_digits(dividend_num));
	workspace_list_to_array(remainder, integer_digits(dividend_num));
	
	workspace_digits zero_digit;
	zero_digit.length = 1;
	fixnum_type z = 0;
	zero_digit.digits = &z;
	
	int remainder_sign = workspace_compare_digits(dividend, zero_digit);
	
	int compare = workspace_compare_digits(remainder, divisor);
	
	while ((compare >= 0) || (remainder_sign == -1)) {
		
		if (compare >= 0) {
			long divisor_power = workspace_highest_digit_index(divisor);
			fixnum_type divisor_first = divisor.digits[divisor_power];
			long remainder_power = workspace_highest_digit_index(remainder);
			fixnum_type remainder_first = remainder.digits[remainder_power];
			fixnum_type remainder_second = (remainder_power == 0) ? 0 : remainder.digits[remainder_power-1];
			fixnum_type remainder_value = remainder_first;
			
			if (divisor_first > remainder_first) {
				remainder_value = remainder_second + remainder_first * integer_base;
				remainder_power--;
			}
			
			long power_difference = remainder_power - divisor_power;
			
			workspace_zero_digits(intermediate_quotient);
			intermediate_quotient.digits[0] = remainder_value / divisor_first;
			workspace_raise_digits(intermediate_quotient, power_difference);
			
			// adjust quotient
			if (remainder_sign == 1) {
				workspace_destructive_addition(quotient, intermediate_quotient);
			}
			else {
				workspace_destructive_subtraction(quotient, intermediate_quotient);
			}
		}
		// remainder negative and abs(remainder) less than divisor
		else if (remainder_sign == -1) {
			workspace_digits one_digit;
			one_digit.length = 1;
			fixnum_type o = 1;
			one_digit.digits = &o;
			workspace_destructive_subtraction(quotient, one_digit);
		}
		else {
			printf("incorrect condition\n");
			exit(1);
		}
		
		// set intermediate to divisor * quotient
		
		workspace_zero_digits(intermediate_result);
		workspace_multiplication(intermediate_result, temporary, divisor, quotient);
		
		remainder_sign = workspace_compare_digits(dividend, intermediate_result);
		
		workspace_zero_digits(remainder);
		
		// set remainder to dividend - intermediate result
		if (remainder_sign == 1) {
			workspace_subtraction(remainder, dividend, intermediate_result);
		}
		else {
			workspace_subtraction(remainder, intermediate_result, dividend);
		}
		
		compare = workspace_compare_digits(remainder, divisor);
	}
	
	long last = workspace_highest_digit_index(quotient);
	
	object* quotient_num;
	if (workspace_digits_are_zero(quotient)) {
		quotient_num = integer_zero();
	}
	else {
		object* quotient_digits = empty_list();
		long i;
		for (i = last; i >= 0; i--) {
			object* num = alloc_fixnum(quotient.digits[i]);
			object* cell = alloc_list_cell(num, quotient_digits);
			quotient_digits = cell;
		}
		
		quotient_num = alloc_integer(1, quotient_digits);
	}
	
	last = workspace_highest_digit_index(remainder);
	
	object* remainder_num;
	if (workspace_digits_are_zero(remainder)) {
		remainder_num = integer_zero();
	}
	else {
		object* remainder_digits = empty_list();
		long i;
		for (i = last; i >= 0; i--) {
			object* num = alloc_fixnum(remainder.digits[i]);
			object* cell = alloc_list_cell(num, remainder_digits);
			remainder_digits = cell;
		}
		
		remainder_num = alloc_integer(1, remainder_digits);
	}
	
	object* ls = alloc_list_2(quotient_num, remainder_num);
	
	return call_cont(cont, ls);
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
	
	object* quot_digits = integer_digits(quot);
	object* rem_digits = integer_digits(rem);
	
	char zero_digits = digits_are_zero(quot_digits);
	object* quotient = zero_digits ? integer_zero() : alloc_integer(fixnum_value(quotient_sign), integer_digits(quot));
	
	char rem_zero = digits_are_zero(rem_digits);
	object* remainder = rem_zero ? integer_zero() : alloc_integer(fixnum_value(remainder_sign), integer_digits(rem));
	object* ls = alloc_list_2(quotient, remainder);
	
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
	
	object* positive_divisor = alloc_positive_integer(integer_digits(divisor));
	object* positive_dividend = alloc_positive_integer(integer_digits(dividend));
	
	object* result_args = alloc_list_2(sign_object(quotient_sign), sign_object(remainder_sign));
	object* result_call = alloc_call(&integer_make_division_result_proc, result_args, cont);
	
	object* divide_args = alloc_list_2(positive_divisor, positive_dividend);
	object* divide_call = alloc_call(&integer_perform_division_proc, divide_args, alloc_cont(result_call));
	
	return perform_call(divide_call);
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
	
	if (integer_is_zero(remainder)) {
		return call_cont(cont, smaller);
	}
	else {
		object* gcd_args = alloc_list_2(remainder, smaller);
		object* gcd_call = alloc_call(&integer_gcd_step_proc, gcd_args, cont);
		
		object* divide_args = alloc_list_2(remainder, smaller);
		object* divide_call = alloc_call(&integer_divide_proc, divide_args, alloc_cont(gcd_call));
		
		return perform_call(divide_call);
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
	
	object* next_call = alloc_call(&integer_quotient_continued_proc, empty_list(), cont);
	object* call = alloc_call(&integer_divide_proc, args, alloc_cont(next_call));
	
	return perform_call(call);
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
	
	object* next_call = alloc_call(&integer_remainder_continued_proc, empty_list(), cont);
	object* call = alloc_call(&integer_divide_proc, args, alloc_cont(next_call));
	
	return perform_call(call);
}

object* integer_greatest_common_divisor(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* a_pos = alloc_positive_integer(integer_digits(a));
	object* b_pos = alloc_positive_integer(integer_digits(b));
	
	int compare = compare_unsigned_integers(a_pos, b_pos);
	
	if (compare == 0) {
		return call_cont(cont, a_pos);
	}
	else {
		char a_smaller = (compare == -1);
		object* smaller = a_smaller ? a_pos : b_pos;
		object* larger = a_smaller ? b_pos : a_pos;
		
		object* quot_rem = alloc_list_2(integer_zero(), smaller);
		object* gcd_args = alloc_list_3(quot_rem, larger, integer_zero());
		object* gcd_call = alloc_call(&integer_gcd_step_proc, gcd_args, cont);
		
		return perform_call(gcd_call);
	}
}

object* integer_subtract_one(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	object* subtract_args = alloc_list_2(integer_one(), a);
	object* subtract_call = alloc_call(&integer_subtract_proc, subtract_args, cont);
	
	return perform_call(subtract_call);
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
	object* cell = alloc_list_cell(num, last);
	
	if (integer_is_zero(quotient)) {
		return call_cont(cont, cell);
	}
	else {
		object* next_args = alloc_list_2(cell, base);
		object* next_call = alloc_call(&integer_digits_to_new_base_proc, next_args, cont);
		
		object* divide_args = alloc_list_2(base, quotient);
		object* divide_call = alloc_call(&integer_divide_proc, divide_args, alloc_cont(next_call));
		
		return perform_call(divide_call);
	}
}

object integer_to_new_base_proc;

// returns a list of digits in the new base, from largest to smallest (opposite to order in integer objects)

object* integer_to_new_base(object* args, object* cont) {
	object* num;
	object* base;
	delist_2(args, &num, &base);
	
	if (integer_is_zero(num)) {
		return call_cont(cont, num);
	}
	else {
		object* make_args = alloc_list_1(sign_object(integer_sign(num)));
		object* make_call = alloc_call(&make_integer_proc, make_args, cont);
		
		object* next_args = alloc_list_2(empty_list(), base);
		object* next_call = alloc_call(&integer_digits_to_new_base_proc, next_args, alloc_cont(make_call));
		
		object* divide_args = alloc_list_2(base, num);
		object* divide_call = alloc_call(&integer_divide_proc, divide_args, alloc_cont(next_call));
		
		return perform_call(divide_call);
	}
}

object* integer_to_decimal(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	object* call_args = alloc_list_2(num, integer_ten());
	object* call = alloc_call(&integer_to_new_base_proc, call_args, cont);
	
	return perform_call(call);
}

object decimal_to_string_proc;

object* decimal_to_string(object* args, object* cont) {
	object* digits;
	object* sign;
	delist_2(args, &digits, &sign);
	
	int length = list_length(digits);
	char negative = fixnum_value(sign) == -1;
	
	int string_length = length + negative;
	
	char* str = alloc_bytes(sizeof(char) * (string_length + 1));
	
	if (negative) {
		str[0] = '-';
	}
	
	int i;
	for (i = !negative; i < string_length; i++) {
		str[i] = fixnum_value(list_first(digits)) - '0';
		digits = list_rest(digits);
	}
	
	str[string_length] = 0;
	object* string = alloc_string(str);
	
	return call_cont(cont, string);
}

object integer_to_string_proc;

object* integer_to_string(object* args, object* cont) {
	object* number;
	delist_1(args, &number);
	
	object* sign = sign_object(integer_sign(number));
	
	object* string_args = alloc_list_1(sign);
	object* string_call = alloc_call(&decimal_to_string_proc, string_args, cont);
	
	object* decimal_call = alloc_call(&integer_to_new_base_proc, args, alloc_cont(string_call));
	
	return perform_call(decimal_call);
}

object print_integer_digits_proc;

object* print_integer_digits(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	object* digits = integer_digits(num);
	
	while (!is_empty_list(digits)) {
		printf("%lld", fixnum_value(list_first(digits)));
		digits = list_rest(digits);
	}
	
	return call_discarding_cont(cont);
}

object* print_integer(object* args, object* cont) {
	object* num;
	delist_1(args, &num);
	
	if (integer_sign(num) == -1) {
		printf("-");
	}
	
	object* print_call = alloc_call(&print_integer_digits_proc, empty_list(), cont);
	object* call = alloc_call(&integer_to_decimal_proc, args, alloc_cont(print_call));
	
	return perform_call(call);
}

void init_integer_procedures(void) {
	init_primitive(&make_integer, &make_integer_proc);
	init_primitive(&integer_compare, &integer_compare_proc);
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
	
	init_primitive(&integer_negate, &integer_negate_proc);

	init_primitive(&integer_multiply, &integer_multiply_proc);
	init_primitive(&integer_multiply_digits, &integer_multiply_digits_proc);
	
	init_primitive(&integer_divide, &integer_divide_proc);
	init_primitive(&integer_make_division_result, &integer_make_division_result_proc);
	init_primitive(&integer_perform_division, &integer_perform_division_proc);
	
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
	
	init_primitive(&print_integer, &print_integer_proc);
	init_primitive(&print_integer_digits, &print_integer_digits_proc);
	add_print_procedure(type_integer, &print_integer_proc);
}
