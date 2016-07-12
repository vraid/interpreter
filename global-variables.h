#pragma once

#include "data-structures.h"

void init_global_variables(void);

object* end_cont(void);

char is_empty_list(object* obj);
char is_empty_stream(object* obj);
char is_empty_sequence(object* obj);
char is_false(object* obj);
char is_true(object* obj);
char is_no_object(object* obj);
char is_no_binding(object* obj);
char is_placeholder_value(object* obj);
char is_empty_struct_definition(object* obj);

object* true(void);
object* false(void);
object* boolean(char b);
object* empty_list(void);
object* empty_stream(void);
object* end_vector_iterator(void);
object* empty_string(void);
object* no_object(void);
object* no_symbol(void);
object* no_binding(void);
object* placeholder_value(void);
object* empty_environment(void);
object* empty_struct_definition(void);

object* quote_symbol(void);

object* dash_string(void);
object* question_mark_string(void);

object* zero(void);
