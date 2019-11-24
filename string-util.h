#pragma once

#include "data-structures.h"

char* copy_str(char* dest, char* str);
int str_length(char* str);

#define alloc_copy_str(a) copy_str(alloc_chars(1 + str_length(a)), a);

object* string_append_2(object* obj, char* str, object* a, object* b);
object* string_append_3(object* obj, char* str, object* a, object* b, object* c);

#define alloc_string_append_2(a, b) string_append_2(alloc_obj(), alloc_chars(1 + string_length(a) + string_length(b)), a, b)
#define alloc_string_append_3(a, b, c) string_append_3(alloc_obj(), alloc_chars(1 + string_length(a) + string_length(b) + string_length(c)), a, b, c)
