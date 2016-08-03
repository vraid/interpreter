#pragma once

#include "data-structures.h"

#define bignum_base_bits 14
#define bignum_base 16384

object make_bignum_proc;
object bignum_add_proc;
object bignum_subtract_proc;
object bignum_subtract_one_proc;
object bignum_multiply_proc;
object bignum_divide_proc;

int compare_signed_bignums(object* a, object* b);
char is_zero_bignum(object* a);

void init_bignum_procedures(void);
