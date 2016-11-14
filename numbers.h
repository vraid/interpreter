#pragma once

#include "data-structures.h"

object number_add_proc;
object number_subtract_proc;
object number_multiply_proc;
object number_divide_proc;
object number_compare_proc;

char is_number(object* obj);
char is_exact_number(object* obj);
char is_exact_real(object* obj);
char is_positive(object* obj);
char is_negative(object* obj);

void init_number_procedures(void);
