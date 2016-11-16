#include "numbers.h"

#include "data-structures.h"
#include "object-init.h"
#include "call.h"
#include "global-variables.h"
#include "delist.h"
#include "integers.h"
#include "fractions.h"

char is_number(object* obj) {
	return is_exact_number(obj);
}
char is_exact_number(object* obj) {
	return is_exact_real(obj);
}
char is_exact_real(object* obj) {
	return is_integer(obj) || is_fraction(obj);
}
char is_positive(object* obj) {
	return (is_fraction(obj) && fraction_is_positive(obj)) || (is_integer(obj) && integer_is_positive(obj));
}
char is_negative(object* obj) {
	return !(is_positive(obj) || integer_is_zero(obj));
}

object* equalize_types(object* integer_proc, object* fraction_proc, object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	if (is_integer(a) && is_integer(b)) {
		object call;
		init_call(&call, integer_proc, args, cont);
		return perform_call(&call);
	}
	else if (is_fraction(a) && is_fraction(b)) {
		object call;
		init_call(&call, fraction_proc, args, cont);
		return perform_call(&call);
	}
	else {
		char a_fraction = is_fraction(a);
		object num;
		init_integral_fraction(&num, a_fraction ? b : a);
		object call_args[2];
		init_list_2(call_args, a_fraction ? a : &num, a_fraction ? &num : b);
		object call;
		init_call(&call, fraction_proc, call_args, cont);
		
		return perform_call(&call);
	}
}

object* number_add(object* args, object* cont) {
	return equalize_types(&integer_add_proc, &fraction_add_proc, args, cont);
}

object* number_subtract(object* args, object* cont) {
	return equalize_types(&integer_subtract_proc, &fraction_subtract_proc, args, cont);
}

object* number_multiply(object* args, object* cont) {
	return equalize_types(&integer_multiply_proc, &fraction_multiply_proc, args, cont);
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
		return equalize_types(&integer_divide_proc, &fraction_divide_proc, args, cont);
	}
}

object* number_compare(object* args, object* cont) {
	return equalize_types(&integer_compare_proc, &fraction_compare_proc, args, cont);
}

void init_number_procedures(void) {
	init_primitive(&number_add, &number_add_proc);
	init_primitive(&number_subtract, &number_subtract_proc);
	init_primitive(&number_multiply, &number_multiply_proc);
	init_primitive(&number_divide, &number_divide_proc);
	init_primitive(&number_compare, &number_compare_proc);
}
