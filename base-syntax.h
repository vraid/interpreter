#pragma once

#include "data-structures.h"

object* syntax_obj(static_syntax type);

object eval_force_proc;

void add_syntax(char* name, static_syntax syntax, primitive_proc* proc);
void init_base_syntax_procedures(void);
