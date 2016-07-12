#pragma once

#include "data-structures.h"

#define generic_args_max 3
object* generic_args[generic_args_max];
object* generic_arg_list[generic_args_max+1];

object* symbol_to_string_func;

void init_standard_functions(void);
