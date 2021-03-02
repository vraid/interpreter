#pragma once

#include "data-structures.h"

object print_entry_proc;
object print_error_entry_proc;
object print_newline_proc;
object print_value_proc;

object print_sequence_element_proc;
object print_first_sequence_element_proc;
object print_sequence_proc;
object print_sequence_end_proc;

object* print_value(object* args, object* cont);

void add_print_procedure(object_type type, object* proc);
void init_print_procedures(void);
