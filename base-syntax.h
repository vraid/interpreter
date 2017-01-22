#pragma once

#include "data-structures.h"

object* syntax_procedure_obj(static_syntax_procedure type);

object eval_force_proc;

void add_syntax(char* name, static_syntax_procedure syntax, primitive_proc* proc);
void init_base_syntax_procedures(void);
