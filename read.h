#pragma once

#include "data-structures.h"

object read_entry_proc;

object read_true_proc;
object read_false_proc;
object read_include_proc;
object read_enter_scope_proc;
object read_rewind_scope_proc;

void init_read_procedures(void);
