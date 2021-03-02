#pragma once

#include "data-structures.h"

#define integer_base_bits 30
#define integer_base 1073741824

object make_integer_proc;
object integer_compare_proc;
object integer_add_proc;
object integer_add_digits_proc;
object integer_add_signless_proc;
object integer_subtract_proc;
object integer_subtract_one_proc;
object integer_multiply_proc;
object integer_multiply_digits_proc;
object integer_divide_proc;
object integer_quotient_proc;
object integer_remainder_proc;
object integer_greatest_common_divisor_proc;
object integer_to_decimal_proc;
object integer_negate_proc;

object print_integer_proc;

object* integer_add(object* args, object* cont);

int compare_signed_integers(object* a, object* b);
char digits_have_value(long long value, object* digits);
char integer_is_zero(object* a);
char integer_is_one(object* a);
char integer_is_positive(object* a);
char integer_is_negative(object* a);
char integers_have_different_signs(object* a, object* b);

void init_integer_procedures(void);
