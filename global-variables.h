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

char is_quote_symbol(object* obj);
char is_define_symbol(object* obj);
char is_lambda_symbol(object* obj);
char is_curry_symbol(object* obj);
char is_apply_symbol(object* obj);
char is_if_symbol(object* obj);
char is_list_symbol(object* obj);
char is_map_symbol(object* obj);
char is_fold_symbol(object* obj);
char is_filter_symbol(object* obj);

object* quote_symbol(void);
object* define_symbol(void);
object* lambda_symbol(void);
object* curry_symbol(void);
object* apply_symbol(void);
object* if_symbol(void);
object* list_symbol(void);
object* map_symbol(void);
object* fold_symbol(void);
object* filter_symbol(void);
