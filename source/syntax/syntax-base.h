#pragma once

#include "data-structures.h"

object* syntax_procedure_obj(static_syntax_procedure type);
context_type syntax_procedure_context(static_syntax_procedure type);

object eval_force_proc;

void set_syntax_properties(static_syntax_procedure syntax, char* name, context_type context);
void init_syntax(static_syntax_procedure syntax, primitive_proc* proc);
void add_syntax(char* name, static_syntax_procedure syntax, context_type context, primitive_proc* proc);
void init_base_syntax_procedures(void);
