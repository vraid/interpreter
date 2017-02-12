#pragma once

#include "data-structures.h"

object print_entry_proc;
object print_error_entry_proc;
object print_newline_proc;

object* print_value(object* args, object* cont);

void init_print_procedures(void);
