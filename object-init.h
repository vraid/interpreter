#pragma once

#include "data-structures.h"

#define alloc_bytes(n) alloca(n)
#define alloc_chars(n) alloc_bytes(sizeof(char) * (n))
#define alloc_objects(n) alloc_bytes(sizeof(object) * (n))
#define alloc_obj() alloc_objects(1)

void make_static(object* obj);
void make_semistatic(object* obj);
object* init_object(object_location loc, object_type t, object* obj);

object* init_string(object* obj, char* value);
#define alloc_string(a) init_string(alloc_obj(), a)

object* init_fixnum(object* obj, long value);
#define alloc_fixnum(a) init_fixnum(alloc_obj(), a)

object* init_integer(object* obj, int sign, object* digits);
#define alloc_integer(a, b) init_integer(alloc_obj(), a, b)

object* init_positive_integer(object* obj, object* digits);
#define alloc_positive_integer(a) init_positive_integer(alloc_obj(), a)

object* init_negated_integer(object* obj, object* num);
#define alloc_negated_integer(a) init_negated_integer(alloc_obj(), a)

object* init_fraction(object* obj, object* numerator, object* denominator);
#define alloc_fraction(a, b) init_fraction(alloc_obj(), a, b)

object* init_integral_fraction(object* obj, object* numerator);
#define alloc_integral_fraction(a) init_integral_fraction(alloc_obj(), a)

object* init_complex(object* obj, object* real, object* imag);
#define alloc_complex(a, b) init_complex(alloc_obj(), a, b)

object* init_symbol(object* obj, object* name);
#define alloc_symbol(a) init_symbol(alloc_obj(), a)

object* init_binding(object* obj, object* name, object* value);
#define alloc_binding(a, b) init_binding(alloc_obj(), a, b)
#define alloc_placeholder_binding(a) init_binding(alloc_obj(), a, placeholder_value())

object* init_stream(object* obj, object* first, object* rest);
#define alloc_stream(a, b) init_stream(alloc_obj(), a, b)

object* init_vector(object* obj, int length, object** data);
#define alloc_vector(a, b) init_vector(alloc_obj(), a, b)

object* init_vector_iterator(object* obj, int n, object* vector);
#define alloc_vector_iterator(a, b) init_vector_iterator(alloc_obj(), a, b)

object* init_module(object* obj, object* bindings);
#define alloc_module(a) init_module(alloc_obj(), a)

object* init_module_interim(object* obj, object* bindings, object* call, object* cont);
#define alloc_module_interim(a, b, c) init_module_interim(alloc_obj(), a, b, c);

object* init_syntax_procedure(object* obj, primitive_proc* proc, static_syntax_procedure id);

object* init_function(object* obj, object* environment, object* parameters, object* body);
#define alloc_function(a, b, c) init_function(alloc_obj(), a, b, c)

object* init_call(object* obj, object* function, object* arguments, object* continuation);
#define alloc_call(a, b, c) init_call(alloc_obj(), a, b, c)

object* init_cont(object* obj, object* call);
#define alloc_cont(a) init_cont(alloc_obj(), a)

object* init_discarding_cont(object* obj, object* call);
#define alloc_discarding_cont(a) init_discarding_cont(alloc_obj(), a)

object* init_catching_cont(object* obj, object* call);
#define alloc_catching_cont(a) init_catching_cont(alloc_obj(), a)

object* init_syntax_object(object* obj, object* syntax, object* origin);
#define alloc_syntax_object(a, b) init_syntax_object(alloc_obj(), a, b)

object* init_internal_position(object* obj, int n);
#define alloc_internal_position(a) init_internal_position(alloc_obj(), a)

object* init_internal_error(object* obj, object* trace, object* message);
#define alloc_internal_error(a, b) init_internal_error(alloc_obj(), a, b)

object* init_reader_entry(object* obj, read_type t, object* proc);
#define alloc_reader_entry(a, b) init_reader_entry(alloc_obj(), a, b)

object* init_eval_context(object* obj, context_type value);
#define alloc_eval_context(a) init_eval_context(alloc_obj(), a)

object* init_memory_reference(object* obj, long size, char* reference);

object* init_file_port(object* obj, FILE* file);

object* init_string_port(object* obj, object* string);
#define alloc_string_port(a) init_string_port(alloc_obj(), a)

object* init_delay(object* obj, object* value);
#define alloc_delay(a) init_delay(alloc_obj(), a)

object* init_primitive_procedure(object* obj, primitive_proc* proc);
object* init_primitive(primitive_proc* proc, object* obj);

object* init_list_cell(object* obj, object* first, object* rest);
#define alloc_list_cell(a, b) init_list_cell(alloc_obj(), a, b)

object* init_list_1(object* ls, object* first);
#define alloc_list_1(a) init_list_1(alloc_objects(1), a)

object* init_list_2(object* ls, object* first, object* second);
#define alloc_list_2(a, b) init_list_2(alloc_objects(2), a, b)

object* init_list_3(object* ls, object* first, object* second, object* third);
#define alloc_list_3(a, b, c) init_list_3(alloc_objects(3), a, b, c)

object* init_list_4(object* ls, object* first, object* second, object* third, object* fourth);
#define alloc_list_4(a, b, c, d) init_list_4(alloc_objects(4), a, b, c, d)

object* init_list_5(object* ls, object* first, object* second, object* third, object* fourth, object* fifth);
#define alloc_list_5(a, b, c, d, e) init_list_5(alloc_objects(5), a, b, c, d, e)

object* init_trace_list(object* cell, object* a, object* trace);
#define alloc_trace_list(a, b) init_trace_list(alloc_obj(), a, b)
