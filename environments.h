#pragma once

#include "data-structures.h"

object bind_values_proc;
object extend_environment_proc;
object bind_and_extend_environment_proc;
object environment_get_proc;

object update_binding_proc;

void init_environment_procedures(void);

object* static_environment(void);
object* add_static_binding(char* name, object* value);

object* extend_environment(object* args, object* cont);
object* find_in_environment(object* env, object* symbol, char return_placeholders);
