#pragma once

#include "data-structures.h"

object make_fraction_proc;
object make_integral_fraction_proc;
object fraction_add_proc;
object fraction_subtract_proc;
object fraction_multiply_proc;
object fraction_divide_proc;

object fraction_quotient_proc;
object fraction_remainder_proc;

object fraction_compare_proc;

object* fraction_add(object* args, object* cont);

char fraction_is_positive(object* obj);

void init_fraction_procedures(void);
