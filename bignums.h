#pragma once

#include "data-structures.h"

#define bignum_base_bits 14
#define bignum_base 16384

object make_bignum_proc;
object bignum_add_proc;
object bignum_add_digits_proc;
object bignum_add_signless_proc;
object bignum_subtract_proc;
object bignum_subtract_one_proc;
object bignum_multiply_proc;
object bignum_multiply_digit_proc;
object bignum_divide_proc;
object bignum_greatest_common_divisor_proc;
object bignum_to_decimal_proc;
object bignum_to_string_proc;

int compare_signed_bignums(object* a, object* b);
char is_zero_bignum(object* a);
char is_positive_bignum(object* a);
char is_negative_bignum(object* a);

void init_bignum_procedures(void);
