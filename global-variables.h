#pragma once

#include "data-structures.h"

void init_global_variables(void);

char is_empty_list(object* obj);
char is_false(object* obj);
char is_true(object* obj);
char is_no_object(object* obj);

object* true(void);
object* false(void);
object* empty_list(void);
object* empty_string(void);
object* no_object(void);
object* no_symbol(void);
object* no_binding(void);
object* empty_environment(void);

object* lambda_symbol(void);
