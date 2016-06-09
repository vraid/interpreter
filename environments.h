#pragma once

#include "data-structures.h"

object extend_environment_proc;

object* static_environment(void);
object* add_static_binding(char* name, object* value);

object* extend_environment(object* args, object* cont);
object* find_in_environment(object* env, object* symbol);

void init_environment_procedures(void);
