#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "data-structures.h"
#include "global-variables.h"

char list_start_delimiter[bracket_type_count] = {'(', '[', '{', '(', 0};
char list_end_delimiter[bracket_type_count] = {')', ']', '}', ')', 0};

void init_type_names(void) {
	type_name[type_none] = "no type";
	type_name[type_boolean] = "boolean";
	type_name[type_string] = "string";
	type_name[type_symbol] = "symbol";
	type_name[type_number] = "number";
	type_name[type_struct_definition] = "struct definition";
	type_name[type_struct_instance] = "struct instance";
	type_name[type_list] = "list";
	type_name[type_stream] = "stream";
	type_name[type_vector] = "vector";
	type_name[type_vector_iterator] = "vector iterator";
	type_name[type_primitive_procedure] = "primitive procedure";
	type_name[type_syntax] = "syntax";
	type_name[type_function] = "function";
	type_name[type_call] = "call";
	type_name[type_continuation] = "continuation";
	type_name[type_binding] = "binding";
	type_name[type_environment] = "environment";
	type_name[type_file_port] = "file port";
	type_name[type_internal_error] = "internal error";
}

void init_location_names(void) {
	location_name[location_none] = "no location";
	location_name[location_stack] = "stack";
	location_name[location_heap] = "heap";
	location_name[location_static] = "static";
	location_name[location_moved] = "moved";
}

void init_data_structure_names(void) {
	init_type_names();
	init_location_names();
}

char* object_type_name(object* obj) {
	return type_name[obj->type];
}

void check_type(object_type type, object* obj) {
	if (obj->type >= type_count) {
		fprintf(stderr, "type out of range at %p, expected %s\n", (void*)obj, type_name[type]);
	}
	if (obj->type != type) {
		fprintf(stderr, "faulty type at %p: expected %s, got %s\n", (void*)obj, type_name[type], type_name[obj->type]);
	}
	return;
}

char is_type(object_type type, object* obj) {
	return obj->type == type;
}

char is_boolean(object* obj) {
	return is_type(type_boolean, obj);
}

char boolean_value(object* obj) {
	check_type(type_boolean, obj);
	return obj->data.boolean.value;
}

char is_string(object* obj) {
	return is_type(type_string, obj);
}

int string_length(object* obj) {
	check_type(type_string, obj);
	return obj->data.string.length;
}
 
char* string_value(object* obj) {
	check_type(type_string, obj);
	return obj->data.string.value;
}

char is_symbol(object* obj) {
	return is_type(type_symbol, obj);
}

object* symbol_name(object* obj) {
	check_type(type_symbol, obj);
	return obj->data.symbol.name;
}

char is_number(object* obj) {
	return is_type(type_number, obj);
}

long number_value(object* obj) {
	check_type(type_number, obj);
	return obj->data.number.value;
}

char is_list(object* obj) {
	return is_type(type_list, obj);
}

bracket_type list_type(object* obj) {
	check_type(type_list, obj);
	return obj->data.list.type;
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
	}
	object* rest = ls->data.list.rest;
	if (!is_list(rest)) {
		fprintf(stderr, "list rest is wrong type: %s\n", type_name[rest->type]);
		return no_object();
	}
	else {
		return rest;
	}
}

char is_nonempty_list(object* obj) {
	return is_list(obj) && !is_empty_list(obj);
}

char is_stream(object* obj) {
	return is_type(type_stream, obj);
}

object* stream_first(object* obj) {
	check_type(type_stream, obj);
	return obj->data.stream.first;
}

object* stream_rest(object* obj) {
	check_type(type_stream, obj);
	return obj->data.stream.rest;
}

char is_sequence(object* obj) {
	switch(obj->type) {
		case type_list:
		case type_stream:
		case type_vector_iterator: return 1;
		default: return 0;
	}
}

char is_vector(object* obj) {
	return is_type(type_vector, obj);
}

int vector_length(object* obj) {
	check_type(type_vector, obj);
	return obj->data.vector.length;
}

object** vector_data(object* obj) {
	check_type(type_vector, obj);
	return obj->data.vector.data;
}

char is_vector_iterator(object* obj) {
	return is_type(type_vector_iterator, obj);
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

char is_struct_definition(object* obj) {
	return is_type(type_struct_definition, obj);
}

object* struct_definition_name(object* obj) {
	check_type(type_struct_definition, obj);
	return obj->data.struct_definition.name;
}

object* struct_definition_fields(object* obj) {
	check_type(type_struct_definition, obj);
	return obj->data.struct_definition.fields;
}

char is_struct_instance(object* obj) {
	return is_type(type_struct_instance, obj);
}

object* struct_instance_type(object* obj) {
	check_type(type_struct_instance, obj);
	return obj->data.struct_instance.type;
}

object* struct_instance_data(object* obj) {
	check_type(type_struct_instance, obj);
	return obj->data.struct_instance.data;
}

char is_syntax(object* obj) {
	return is_type(type_syntax, obj);
}

primitive_proc* syntax_proc(object* obj) {
	check_type(type_syntax, obj);
	return obj->data.syntax.proc;
}

char is_function(object* obj) {
	return is_type(type_function, obj);
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

char is_primitive_procedure(object* obj) {
	return obj->type == type_primitive_procedure;
}

primitive_proc* primitive_procedure_proc(object* obj) {
	check_type(type_primitive_procedure, obj);
	return obj->data.primitive_procedure.proc;
}

char is_binding(object* obj) {
	return obj->type == type_binding;
}

object* binding_name(object* obj) {
	check_type(type_binding, obj);
	return obj->data.binding.name;
}

object* binding_value(object* obj) {
	check_type(type_binding, obj);
	return obj->data.binding.value;
}

char is_environment(object* obj) {
	return obj->type == type_environment;
}

object* environment_bindings(object* obj) {
	check_type(type_environment, obj);
	return obj->data.environment.bindings;
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

object* continuation_call(object* obj) {
	check_type(type_continuation, obj);
	return obj->data.continuation.call;
}

char is_file_port(object* obj) {
	return obj->type == type_file_port;
}

FILE* file_port_file(object* obj) {
	check_type(type_file_port, obj);
	return obj->data.file_port.file;
}

char is_call(object* obj) {
	return obj->type == type_call;
}

char is_continuation(object* obj) {
	return obj->type == type_continuation;
}

cont_type continuation_type(object* obj) {
	check_type(type_continuation, obj);
	return obj->data.continuation.conttype;
}

char is_discarding_continuation(object* obj) {
	return continuation_type(obj) == cont_discarding;
}

char is_catching_continuation(object* obj) {
	return continuation_type(obj) == cont_catching;
}

char is_internal_error(object* obj) {
	return obj->type == type_internal_error;
}

object* internal_error_message(object* obj) {
	check_type(type_internal_error, obj);
	return obj->data.internal_error.message;
}

char is_delay(object* obj) {
	return obj->type == type_delay;
}

object* delay_value(object* obj) {
	check_type(type_delay, obj);
	return obj->data.delay.value;
}

object* delay_environment(object* obj) {
	check_type(type_delay, obj);
	return obj->data.delay.environment;
}

char delay_evaluated(object* obj) {
	check_type(type_delay, obj);
	return obj->data.delay.evaluated;
}

int list_length(object* ls) {
	int n = 0;
	while (!is_empty_list(ls)) {
		n++;
		ls = list_rest(ls);
	}
	return n;
}

char list_starts_with(object* ls, object* obj) {
	return is_nonempty_list(ls) && (obj == list_first(ls));
}
