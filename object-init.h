#pragma once

#include "data-structures.h"

void init_object(object_location loc, object_type t, object* obj);
void init_string(object* obj, char* value);
void init_symbol(object* obj, object* name);
void init_primitive_procedure(object* obj, primitive_proc* proc);
void init_call(object* obj, object* function, object* arguments, object* continuation);
void init_cont(object* obj, object* call);
void init_discarding_cont(object* obj, object* call);

void init_list_cell(object* obj, object* first, object* rest);
void init_list_1(object* ls, object* first);
void init_list_2(object* ls, object* first, object* second);
void init_list_3(object* ls, object* first, object* second, object* third);
