#pragma once

#include "data-structures.h"

#define generic_args_max 3
object* generic_args[generic_args_max];
object* generic_arg_list[generic_args_max+1];

void init_standard_functions(void);

object* first_func(void);
object* rest_func(void);
