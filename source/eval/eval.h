#pragma once

#include "data-structures.h"

object eval_proc;
object eval_with_environment_proc;
object eval_function_call_proc;
object eval_list_elements_proc;

void init_eval_procedures(void);

object* eval(object* args, object* cont);
