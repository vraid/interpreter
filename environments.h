#pragma once

#include "data-structures.h"

object extend_environment_proc;

object* bind_values_proc(void);

object* static_environment(void);
object* add_static_binding(object* value, char* name);

object* extend_environment(object* args, object* cont);
object* find_in_environment(object* env, object* symbol);

void init_environment_procedures(void);
