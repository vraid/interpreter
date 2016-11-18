#include "numbers.h"

#include <stdlib.h>
#include "data-structures.h"
#include "object-init.h"
#include "call.h"
#include "global-variables.h"
#include "delist.h"
#include "integers.h"
#include "fractions.h"
#include "complex.h"

char is_number(object* obj) {
	return is_exact_number(obj);
}
char is_exact_number(object* obj) {
	return is_exact_complex(obj);
}
char is_exact_complex(object* obj) {
	return is_complex(obj) || is_exact_real(obj);
}
char is_exact_real(object* obj) {
	return is_integer(obj) || is_fraction(obj);
}
char is_exact_imaginary(object* obj) {
	return integer_is_zero(number_real_part(obj));
}
char real_sign(object* obj) {
	if (is_integer(obj)) {
		return integer_sign(obj);
	}
	else if (is_fraction(obj)) {
		return fraction_sign(obj);
	}
	else {
		printf("sign of non-real\n");
		exit(1);
	}
}
char is_positive(object* obj) {
	return real_sign(obj) == 1;
}
char is_negative(object* obj) {
	return real_sign(obj) == -1;
}
char number_is_zero(object* obj) {
	return is_integer(obj) && integer_is_zero(obj);
}
object* number_real_part(object* obj) {
	return is_complex(obj) ? complex_real_part(obj) : obj;
}
object* number_imag_part(object* obj) {
	return is_complex(obj) ? complex_imag_part(obj) : integer_zero();
}

object* equalize_up_to_fraction(object* integer_proc, object* fraction_proc, object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	char a_fraction = is_fraction(a);
	char b_fraction = is_fraction(b);
	
	if (a_fraction || b_fraction) {
		if (a_fraction && b_fraction) {
			object call;
			init_call(&call, fraction_proc, args, cont);
			return perform_call(&call);
		}
		else {
			object num;
			init_integral_fraction(&num, a_fraction ? b : a);
			object call_args[2];
			init_list_2(call_args, a_fraction ? a : &num, a_fraction ? &num : b);
			object call;
			init_call(&call, fraction_proc, call_args, cont);
			
			return perform_call(&call);
		}
	}
	else {
		object call;
		init_call(&call, integer_proc, args, cont);
		return perform_call(&call);
	}
}

object* equalize_up_to_complex(object* integer_proc, object* fraction_proc, object* complex_proc, object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	char a_complex = is_complex(a);
	char b_complex = is_complex(b);
	if (a_complex || b_complex) {
		if (a_complex && b_complex) {
			object call;
			init_call(&call, complex_proc, args, cont);
			return perform_call(&call);
		}
		else {
			object num;
			init_complex(&num, a_complex ? b : a, integer_zero());
			object call_args[2];
			init_list_2(call_args, a_complex ? a : &num, a_complex ? &num : b);
			object call;
			init_call(&call, complex_proc, call_args, cont);
			
			return perform_call(&call);
		}
	}
	else {
		return equalize_up_to_fraction(integer_proc, fraction_proc, args, cont);
	}
}

object* number_add(object* args, object* cont) {
	return equalize_up_to_complex(&integer_add_proc, &fraction_add_proc, &complex_add_proc, args, cont);
}

object* number_subtract(object* args, object* cont) {
	return equalize_up_to_complex(&integer_subtract_proc, &fraction_subtract_proc, &complex_subtract_proc, args, cont);
}

object* number_multiply(object* args, object* cont) {
	return equalize_up_to_complex(&integer_multiply_proc, &fraction_multiply_proc, &complex_multiply_proc, args, cont);
}

object* number_divide(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (is_integer(a) && is_integer(b)) {
		object call_args[2];
		init_list_2(call_args, b, a);
		object call;
		init_call(&call, &make_fraction_proc, call_args, cont);
		return perform_call(&call);
	}
	else {
		return equalize_up_to_fraction(&integer_divide_proc, &fraction_divide_proc, args, cont);
	}
}

object* number_compare(object* args, object* cont) {
	return equalize_up_to_fraction(&integer_compare_proc, &fraction_compare_proc, args, cont);
}

void init_number_procedures(void) {
	init_primitive(&number_add, &number_add_proc);
	init_primitive(&number_subtract, &number_subtract_proc);
	init_primitive(&number_multiply, &number_multiply_proc);
	init_primitive(&number_divide, &number_divide_proc);
	init_primitive(&number_compare, &number_compare_proc);
}
