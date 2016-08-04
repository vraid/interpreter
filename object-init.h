#pragma once

#include "data-structures.h"

void make_static(object* obj);
void init_object(object_location loc, object_type t, object* obj);
void init_string(object* obj, char* value);
void init_fixnum(object* obj, long value);
void init_bignum(object* obj, int sign, object* digits);
void init_positive_bignum(object* obj, object* digits);
void init_symbol(object* obj, object* name);
void init_binding(object* obj, object* name, object* value);
void init_stream(object* obj, object* first, object* rest);
void init_vector(object* obj, int length, object** data);
void init_vector_iterator(object* obj, int n, object* vector);
void init_struct_definition(object* obj, object* name, object* fields, object* constructor, object* parent);
void init_struct_instance(object* obj, object* type, object* data);
void init_environment(object* obj, object* bindings);
void init_syntax(object* obj, primitive_proc* proc);
void init_primitive_procedure(object* obj, primitive_proc* proc);
void init_function(object* obj, object* environment, object* parameters, object* body);
void init_call(object* obj, object* function, object* arguments, object* continuation);
void init_cont(object* obj, object* call);
void init_discarding_cont(object* obj, object* call);
void init_catching_cont(object* obj, object* call);
void init_internal_error(object* obj, object* message);
void init_delay(object* obj, object* value, object* environment);

void init_list_cell(object* obj, object* first, object* rest);
void init_list_1(object* ls, object* first);
void init_list_2(object* ls, object* first, object* second);
void init_list_3(object* ls, object* first, object* second, object* third);
void init_list_4(object* ls, object* first, object* second, object* third, object* fourth);
void init_list_5(object* ls, object* first, object* second, object* third, object* fourth, object* fifth);
