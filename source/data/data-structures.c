#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "data-structures.h"
#include "global-variables.h"

char* type_names[type_count];

char* type_name(object_type type) {
	return type_names[type];
}

char* object_type_name(object* obj) {
	return type_name(obj->type);
}

void init_type_names(void) {
	type_names[type_none] = "no type";
	type_names[type_boolean] = "boolean";
	type_names[type_string] = "string";
	type_names[type_symbol] = "symbol";
	type_names[type_fixnum] = "fixnum";
	type_names[type_integer] = "integer";
	type_names[type_fraction] = "fraction";
	type_names[type_complex] = "complex";
	type_names[type_module] = "module";
	type_names[type_module_interim] = "module interim";
	type_names[type_list] = "list";
	type_names[type_stream] = "stream";
	type_names[type_vector] = "vector";
	type_names[type_vector_iterator] = "vector iterator";
	type_names[type_primitive_procedure] = "primitive procedure";
	type_names[type_syntax_procedure] = "syntax procedure";
	type_names[type_function] = "function";
	type_names[type_call] = "call";
	type_names[type_continuation] = "continuation";
	type_names[type_binding] = "binding";
	type_names[type_file_port] = "file port";
	type_names[type_string_port] = "string port";
	type_names[type_syntax_object] = "syntax object";
	type_names[type_internal_position] = "internal position";
	type_names[type_internal_error] = "internal error";
}

void init_location_names(void) {
	location_name[location_none] = "no location";
	location_name[location_stack] = "stack";
	location_name[location_heap] = "heap";
	location_name[location_semistatic] = "semistatic";
	location_name[location_static] = "static";
	location_name[location_moved] = "moved";
}

void init_data_structure_names(void) {
	init_type_names();
	init_location_names();
}

void check_type(object_type type, object* obj) {
	if (obj->type >= type_count) {
		fprintf(stderr, "type out of range at %p, expected %s\n", (void*)obj, type_name(type));
		exit(1);
	}
	if (obj->type != type) {
		fprintf(stderr, "faulty type at %p: expected %s, got %s\n", (void*)obj, type_name(type), object_type_name(obj));
		exit(1);
	}
	return;
}

char is_type(object_type type, object* obj) {
	return obj->type == type;
}
char is_boolean(object* obj) {
	return is_type(type_boolean, obj);
}
char is_string(object* obj) {
	return is_type(type_string, obj);
}
char is_symbol(object* obj) {
	return is_type(type_symbol, obj);
}
char is_fixnum(object* obj) {
	return is_type(type_fixnum, obj);
}
char is_integer(object* obj) {
	return is_type(type_integer, obj);
}
char is_fraction(object* obj) {
	return is_type(type_fraction, obj);
}
char is_complex(object* obj) {
	return is_type(type_complex, obj);
}
char is_list(object* obj) {
	return is_type(type_list, obj);
}
char is_nonempty_list(object* obj) {
	return is_list(obj) && !is_empty_list(obj);
}
char is_stream(object* obj) {
	return is_type(type_stream, obj);
}
char is_vector(object* obj) {
	return is_type(type_vector, obj);
}
char is_vector_iterator(object* obj) {
	return is_type(type_vector_iterator, obj);
}
char is_module(object* obj) {
	return is_type(type_module, obj);
}
char is_module_interim(object* obj) {
	return is_type(type_module_interim, obj);
}
char is_syntax_procedure(object* obj) {
	return is_type(type_syntax_procedure, obj);
}
char is_function(object* obj) {
	return is_type(type_function, obj);
}
char is_primitive_procedure(object* obj) {
	return obj->type == type_primitive_procedure;
}
char is_binding(object* obj) {
	return obj->type == type_binding;
}
char is_file_port(object* obj) {
	return obj->type == type_file_port;
}
char is_string_port(object* obj) {
	return obj->type == type_string_port;
}
char is_eval_context(object* obj) {
	return obj->type == type_eval_context;
}
char is_memory_reference(object* obj) {
	return obj->type == type_memory_reference;
}
char is_call(object* obj) {
	return obj->type == type_call;
}
char is_continuation(object* obj) {
	return obj->type == type_continuation;
}
char is_discarding_continuation(object* obj) {
	return continuation_type(obj) == cont_discarding;
}
char is_catching_continuation(object* obj) {
	return continuation_type(obj) == cont_catching;
}
char is_syntax_object(object* obj) {
	return obj->type == type_syntax_object;
}
char is_internal_position(object* obj) {
	return obj->type == type_internal_position;
}
char is_internal_error(object* obj) {
	return obj->type == type_internal_error;
}
char is_delay(object* obj) {
	return obj->type == type_delay;
}

char boolean_value(object* obj) {
	check_type(type_boolean, obj);
	return obj->data.boolean.value;
}
int string_length(object* obj) {
	check_type(type_string, obj);
	return obj->data.string.length;
}
char* string_value(object* obj) {
	check_type(type_string, obj);
	return obj->data.string.value;
}
object* symbol_name(object* obj) {
	check_type(type_symbol, obj);
	return obj->data.symbol.name;
}
FILE* file_port_file(object* obj) {
	check_type(type_file_port, obj);
	return obj->data.file_port.file;
}
int file_port_position(object* obj) {
	check_type(type_file_port, obj);
	return obj->data.file_port.position;
}
int file_port_last_read_char(object* obj) {
	check_type(type_file_port, obj);
	return obj->data.file_port.last_read_char;
}

object* string_port_string(object* obj) {
	check_type(type_string_port, obj);
	return obj->data.string_port.string;
}
int string_port_position(object* obj) {
	check_type(type_string_port, obj);
	return obj->data.string_port.position;
}

read_type reader_entry_read_type(object* obj) {
	check_type(type_reader_entry, obj);
	return obj->data.reader_entry.read_type;
}
object* reader_entry_proc(object* obj) {
	check_type(type_reader_entry, obj);
	return obj->data.reader_entry.proc;
}

context_type eval_context_value(object* obj) {
	check_type(type_eval_context, obj);
	return obj->data.eval_context.value;
}

long memory_reference_size(object* obj) {
	check_type(type_memory_reference, obj);
	return obj->data.memory_reference.size;
}

char* memory_reference_value(object* obj) {
	check_type(type_memory_reference, obj);
	return obj->data.memory_reference.value;
}

fixnum_type fixnum_value(object* obj) {
	check_type(type_fixnum, obj);
	return obj->data.fixnum.value;
}
int integer_sign(object* obj) {
	check_type(type_integer, obj);
	return obj->data.integer.sign;
}
object* integer_digits(object* obj) {
	check_type(type_integer, obj);
	return obj->data.integer.digits;
}
object* fraction_numerator(object* obj) {
	check_type(type_fraction, obj);
	return obj->data.fraction.numerator;
}
object* fraction_denominator(object* obj) {
	check_type(type_fraction, obj);
	return obj->data.fraction.denominator;
}
object* complex_real_part(object* obj) {
	check_type(type_complex, obj);
	return obj->data.complex.real_part;
}
object* complex_imag_part(object* obj) {
	check_type(type_complex, obj);
	return obj->data.complex.imag_part;
}

object* list_first(object* ls) {
	check_type(type_list, ls);
	if (is_empty_list(ls)) {
		fprintf(stderr, "first on empty list\n");
	}
	return ls->data.list.first;
}
object* list_rest(object* ls) {
	check_type(type_list, ls);
	if (is_empty_list(ls)) {
		fprintf(stderr, "rest on empty list\n");
		return no_object();
	}
	object* rest = ls->data.list.rest;
	if (!is_list(rest)) {
		fprintf(stderr, "list rest is wrong type: %s\n", object_type_name(rest));
		return no_object();
	}
	else {
		return rest;
	}
}
object* list_rest_or_empty(object* ls) {
	check_type(type_list, ls);
	return is_empty_list(ls) ? empty_list() : list_rest(ls);
}
object* stream_first(object* obj) {
	check_type(type_stream, obj);
	return obj->data.stream.first;
}
object* stream_rest(object* obj) {
	check_type(type_stream, obj);
	return obj->data.stream.rest;
}
int vector_length(object* obj) {
	check_type(type_vector, obj);
	return obj->data.vector.length;
}
object** vector_data(object* obj) {
	check_type(type_vector, obj);
	return obj->data.vector.data;
}
int vector_iterator_index(object* obj) {
	check_type(type_vector_iterator, obj);
	return obj->data.vector_iterator.n;
}
object* vector_iterator_vector(object* obj) {
	check_type(type_vector_iterator, obj);
	return obj->data.vector_iterator.vector;
}
int vector_iterator_length(object* obj) {
	check_type(type_vector_iterator, obj);
	return vector_length(vector_iterator_vector(obj)) - vector_iterator_index(obj);
}
object* delay_value(object* obj) {
	check_type(type_delay, obj);
	return obj->data.delay.value;
}
char delay_evaluated(object* obj) {
	check_type(type_delay, obj);
	return obj->data.delay.evaluated;
}

object* module_bindings(object* obj) {
	check_type(type_module, obj);
	return obj->data.module.bindings;
}

object* module_interim_bindings(object* obj) {
	check_type(type_module_interim, obj);
	return obj->data.module_interim.bindings;
}
object* module_interim_call(object* obj) {
	check_type(type_module_interim, obj);
	return obj->data.module_interim.call;
}
object* module_interim_cont(object* obj) {
	check_type(type_module_interim, obj);
	return obj->data.module_interim.cont;
}

object* function_parameters(object* obj) {
	check_type(type_function, obj);
	return obj->data.function.parameters;
}
object* function_environment(object* obj) {
	check_type(type_function, obj);
	return obj->data.function.environment;
}
object* function_body(object* obj) {
	check_type(type_function, obj);
	return obj->data.function.body;
}
primitive_proc* syntax_procedure_proc(object* obj) {
	check_type(type_syntax_procedure, obj);
	return obj->data.syntax_procedure.proc;
}
static_syntax_procedure syntax_procedure_id(object* obj) {
	check_type(type_syntax_procedure, obj);
	return obj->data.syntax_procedure.id;
}
primitive_proc* primitive_procedure_proc(object* obj) {
	check_type(type_primitive_procedure, obj);
	return obj->data.primitive_procedure.proc;
}
object* call_function(object* obj) {
	check_type(type_call, obj);
	return obj->data.call.function;
}
object* call_arguments(object* obj) {
	check_type(type_call, obj);
	return obj->data.call.arguments;
}
object* call_continuation(object* obj) {
	check_type(type_call, obj);
	return obj->data.call.continuation;
}
cont_type continuation_type(object* obj) {
	check_type(type_continuation, obj);
	return obj->data.continuation.conttype;
}
object* continuation_call(object* obj) {
	check_type(type_continuation, obj);
	return obj->data.continuation.call;
}

object* binding_name(object* obj) {
	check_type(type_binding, obj);
	return obj->data.binding.name;
}
object* binding_value(object* obj) {
	check_type(type_binding, obj);
	return obj->data.binding.value;
}

object* syntax_object_syntax(object* obj) {
	check_type(type_syntax_object, obj);
	return obj->data.syntax_object.syntax;
}
object* syntax_object_origin(object* obj) {
	check_type(type_syntax_object, obj);
	return obj->data.syntax_object.origin;
}

object* syntax_object_position(object* obj) {
	while (!is_internal_position(obj)) {
		check_type(type_syntax_object, obj);
		obj = syntax_object_origin(obj);
	}
	check_type(type_internal_position, obj);
	return obj;
}

object* desyntax(object* obj) {
	obj = is_syntax_object(obj) ? syntax_object_syntax(obj) : obj;
	if (is_syntax_object(obj)) {
		fprintf(stderr, "syntax containing syntax\n");
	}
	return obj;
}

int internal_position_n(object* obj) {
	check_type(type_internal_position, obj);
	return obj->data.internal_position.n;
}

object* internal_error_trace(object* obj) {
	check_type(type_internal_error, obj);
	return obj->data.internal_error.trace;
}

object* internal_error_message(object* obj) {
	check_type(type_internal_error, obj);
	return obj->data.internal_error.message;
}
