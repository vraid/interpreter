#pragma once

#include "data-structures.h"

void init_eval_procedures(void);

object* eval_proc(void);
object* eval(object* args, object* cont);
object* eval_function_proc(void);

object* eval_list_elements_proc(void);
object* eval_with_environment_proc(void);
