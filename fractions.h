#pragma once

#include "data-structures.h"

object make_fraction_proc;
object fraction_add_proc;
object fraction_subtract_proc;
object fraction_multiply_proc;
object fraction_divide_proc;

object fraction_quotient_proc;
object fraction_remainder_proc;

object fraction_compare_proc;

char is_zero_fraction(object* obj);
char is_integral_fraction(object* obj);

void init_fraction_procedures(void);
