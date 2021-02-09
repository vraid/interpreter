#pragma once

#include "data-structures.h"

object number_add_proc;
object number_subtract_proc;
object number_multiply_proc;
object number_divide_proc;
object number_compare_proc;
object number_negate_proc;
object number_conjugate_proc;

char real_sign(object* obj);
char is_number(object* obj);
char is_exact_number(object* obj);
char is_exact_complex(object* obj);
char is_exact_real(object* obj);
char is_exact_imaginary(object* obj);
char is_positive(object* obj);
char is_negative(object* obj);
char number_is_zero(object* obj);
object* number_real_part(object* obj);
object* number_imag_part(object* obj);

void init_number_procedures(void);
