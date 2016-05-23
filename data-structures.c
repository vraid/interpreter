#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "data-structures.h"
#include "allocation.h"
#include "global-variables.h"

char list_start_delimiter[bracket_type_count] = {'(', '[', '{', '(', 0};
char list_end_delimiter[bracket_type_count] = {')', ']', '}', ')', 0};

void init_type_names(void) {
	type_name[type_none] = "no-type";
	type_name[type_boolean] = "boolean";
	type_name[type_symbol] = "symbol";
	type_name[type_number] = "number";
	type_name[type_list] = "list";
	type_name[type_primitive_procedure] = "primitive_procedure";
	type_name[type_function] = "function";
	type_name[type_call] = "call";
	type_name[type_continuation] = "continuation";
	type_name[type_binding] = "binding";
	type_name[type_environment] = "environment";
	type_name[type_file_port] = "file_port";
}

char* object_type_name(object* obj) {
	return type_name[obj->type];
}

void check_type(object_type type, object* obj) {
	if (obj->type != type) {
		fprintf(stderr, "faulty type: expected ");
		fprintf(stderr, type_name[type]);
		fprintf(stderr, ", got ");
		fprintf(stderr, type_name[obj->type]);
		fprintf(stderr, " \n");
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

char boolean_value(object* obj) {
	check_type(type_boolean, obj);
	return obj->data.boolean.value;
}

char is_symbol(object* obj) {
	return is_type(type_symbol, obj);
}

char* symbol_name(object* obj) {
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
	return ls->data.list.first;
}

object* list_rest(object* ls) {
	check_type(type_list, ls);
	return ls->data.list.rest;
}

char is_nonempty_list(object* obj) {
	return is_list(obj) && !is_empty_list(obj);
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
	return obj->type = type_continuation;
}
char is_discarding_continuation(object* obj) {
	return (is_continuation(obj) && obj->data.continuation.discard_argument);
}

int list_length(object* ls) {
	int n = 0;
	while (!is_empty_list(ls)) {
		n++;
		ls = list_rest(ls);
	}
	return n;
}

char is_self_quoting(object* obj) {
	switch(obj->type) {
		case type_boolean:
		case type_number:
			return 1;
		default:
			return 0;
	}
}

char list_starts_with(object* ls, object* obj) {
	return is_nonempty_list(ls) && (obj == list_first(ls));
}

object* make_number(long value) {
	object* obj = allocate_object(type_number);
	obj->data.number.value = value;
	return obj;
}
