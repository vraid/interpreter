#pragma once

#include "data-structures.h"

#define integer_base_bits 14
#define integer_base 16384

object make_integer_proc;
object integer_add_proc;
object integer_add_digits_proc;
object integer_add_signless_proc;
object integer_subtract_proc;
object integer_subtract_one_proc;
object integer_multiply_proc;
object integer_multiply_digit_proc;
object integer_divide_proc;
object integer_greatest_common_divisor_proc;
object integer_to_decimal_proc;
object integer_to_string_proc;

int compare_signed_integers(object* a, object* b);
char is_zero_integer(object* a);
char is_positive_integer(object* a);
char is_negative_integer(object* a);

void init_integer_procedures(void);