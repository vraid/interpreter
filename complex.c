#include "complex.h"

#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"
#include "eval.h"
#include "numbers.h"

object* make_complex(object* args, object* cont) {
	object* real;
	object* imaginary;
	delist_2(args, &real, &imaginary);
	
	if (number_is_zero(imaginary)) {
		return call_cont(cont, real);
	}
	else {
		object complex;
		init_complex(&complex, real, imaginary);
		return call_cont(cont, &complex);
	}
}

object* complex_add_sub(object* add_sub_proc, object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object real_list[3];
	init_list_3(real_list, add_sub_proc, complex_real_part(a), complex_real_part(b));
	object imag_list[3];
	init_list_3(imag_list, add_sub_proc, complex_imag_part(a), complex_imag_part(b));
	object complex_list[3];
	init_list_3(complex_list, &make_complex_proc, real_list, imag_list);
	
	object eval_args[2];
	init_list_2(eval_args, empty_environment(), complex_list);
	object eval_call;
	init_call(&eval_call, &eval_with_environment_proc, eval_args, cont);
	
	return perform_call(&eval_call);
}

object* complex_add(object* args, object* cont) {
	return complex_add_sub(&number_add_proc, args, cont);
}

object* complex_subtract(object* args, object* cont) {
	return complex_add_sub(&number_subtract_proc, args, cont);
}

object* complex_negate(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	object real_list[2];
	init_list_2(real_list, &number_negate_proc, complex_real_part(a));
	object imag_list[2];
	init_list_2(imag_list, &number_negate_proc, complex_imag_part(a));
	
	object make_list[3];
	init_list_3(make_list, &make_complex_proc, real_list, imag_list);
	
	object eval_args[2];
	init_list_2(eval_args, empty_environment(), make_list);
	object eval_call;
	init_call(&eval_call, &eval_with_environment_proc, eval_args, cont);
	
	return perform_call(&eval_call);
}

object* complex_multiply(object* args, object* cont) {
	object* a;
	object* b;
	delist_2(args, &a, &b);
	
	object* a_real = complex_real_part(a);
	object* a_imag = complex_imag_part(a);
	object* b_real = complex_real_part(b);
	object* b_imag = complex_imag_part(b);
	object* m = &number_multiply_proc;
	
	object a_list[3];
	init_list_3(a_list, m, a_real, b_real);
	object b_list[3];
	init_list_3(b_list, m, a_imag, b_imag);
	object real_list[3];
	init_list_3(real_list, &number_subtract_proc, b_list, a_list);
	
	object c_list[3];
	init_list_3(c_list, m, a_real, b_imag);
	object d_list[3];
	init_list_3(d_list, m, a_imag, b_real);
	object imag_list[3];
	init_list_3(imag_list, &number_add_proc, c_list, d_list);
	
	object complex_list[3];
	init_list_3(complex_list, &make_complex_proc, real_list, imag_list);
	
	object eval_args[2];
	init_list_2(eval_args, empty_environment(), complex_list);
	object eval_call;
	init_call(&eval_call, &eval_with_environment_proc, eval_args, cont);
	
	return perform_call(&eval_call);
}

object complex_conjugate_two_proc;

object* complex_conjugate_two(object* args, object* cont) {
	object* imag;
	object* real;
	delist_2(args, &imag, &real);
	
	object make_args[2];
	init_list_2(make_args, real, imag);
	object make_call;
	init_call(&make_call, &make_complex_proc, make_args, cont);
	
	return perform_call(&make_call);
}

object* complex_conjugate(object* args, object* cont) {
	object* a;
	delist_1(args, &a);
	
	object next_args[1];
	init_list_1(next_args, complex_real_part(a));
	object next_call;
	init_call(&next_call, &complex_conjugate_two_proc, next_args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	
	object negate_args[1];
	init_list_1(negate_args, complex_imag_part(a));
	object negate_call;
	init_call(&negate_call, &number_negate_proc, negate_args, &next_cont);
	
	return perform_call(&negate_call);
}

object complex_divide_two_proc;

object* complex_divide_two(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	object real_list[3];
	init_list_3(real_list, &number_divide_proc, divisor, complex_real_part(dividend));
	
	object imag_list[3];
	init_list_3(imag_list, &number_divide_proc, divisor, complex_imag_part(dividend));
	
	object make_list[3];
	init_list_3(make_list, &make_complex_proc, real_list, imag_list);
	
	object eval_args[2];
	init_list_2(eval_args, empty_environment(), make_list);
	object eval_call;
	init_call(&eval_call, &eval_with_environment_proc, eval_args, cont);
	
	return perform_call(&eval_call);
}

object* complex_divide(object* args, object* cont) {
	object* divisor;
	object* dividend;
	delist_2(args, &divisor, &dividend);
	
	object* real = complex_real_part(divisor);
	object* imag = complex_imag_part(divisor);
	
	object real_list[3];
	init_list_3(real_list, &number_multiply_proc, real, real);
	object imag_list[3];
	init_list_3(imag_list, &number_multiply_proc, imag, imag);
	
	object divisor_list[3];
	init_list_3(divisor_list, &number_add_proc, real_list, imag_list);
	
	object conjugate_list[2];
	init_list_2(conjugate_list, &complex_conjugate_proc, divisor);
	object dividend_list[3];
	init_list_3(dividend_list, &number_multiply_proc, dividend, conjugate_list);

	object next_list[3];
	init_list_3(next_list, &complex_divide_two_proc, divisor_list, dividend_list);
	
	object eval_args[2];
	init_list_2(eval_args, empty_environment(), next_list);
	object eval_call;
	init_call(&eval_call, &eval_with_environment_proc, eval_args, cont);
	
	return perform_call(&eval_call);
}

void init_complex_procedures(void) {
	init_primitive(&make_complex, &make_complex_proc);
	init_primitive(&complex_add, &complex_add_proc);
	init_primitive(&complex_subtract, &complex_subtract_proc);
	init_primitive(&complex_negate, &complex_negate_proc);
	init_primitive(&complex_multiply, &complex_multiply_proc);
	init_primitive(&complex_divide, &complex_divide_proc);
	init_primitive(&complex_divide_two, &complex_divide_two_proc);
	init_primitive(&complex_conjugate, &complex_conjugate_proc);
	init_primitive(&complex_conjugate_two, &complex_conjugate_two_proc);
}