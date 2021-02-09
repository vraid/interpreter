#pragma once

#include "data-structures.h"

void init_global_variables(void);

object* end_cont(void);

char is_empty_list(object* obj);
char is_empty_stream(object* obj);
char is_empty_sequence(object* obj);
char is_false(object* obj);
char is_true(object* obj);
char is_no_symbol(object* obj);
char is_no_object(object* obj);
char is_no_binding(object* obj);
char is_placeholder_value(object* obj);
char is_zero(object* obj);

object* nothing(void);
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

object* define_symbol(void);
object* quote_symbol(void);

object* dash_string(void);
object* question_mark_string(void);

object* zero(void);
object* one(void);
object* negative_one(void);
object* sign_object(int sign);
object* ten(void);

object* integer_zero_list(void);
object* integer_zero(void);
object* integer_one_list(void);
object* integer_one(void);
object* integer_ten_list(void);
object* integer_ten(void);

object* default_context(void);
object* module_context(void);
object* scope_context(void);
object* repl_context(void);
