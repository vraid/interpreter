#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "data-structures.h"
#include "allocation.h"
#include "global-variables.h"

char list_start_delimiter[bracket_type_count] = {'(', '[', '{', '('};
char list_end_delimiter[bracket_type_count] = {')', ']', '}', ')'};

void check_type(object_type type, object* obj) {
	if (obj->type != type) {
		fprintf(stderr, "faulty type\n");
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

object* primitive_procedure_parameters(object* obj) {
	check_type(type_primitive_procedure, obj);
	return obj->data.primitive_procedure.parameters;
}

primitive_proc primitive_procedure_proc(object* obj) {
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

object* make_empty_list(bracket_type type) {
	object* obj = allocate_list_type(type);
	obj->data.list.first = no_object();
	obj->data.list.rest = obj;
	return obj;
}

object* make_binding(object* name, object* value) {
	object* obj = allocate_object_type(type_binding);
	obj->data.binding.name = name;
	obj->data.binding.value = value;
	return obj;
}

object* make_environment(object* bindings) {
	object* obj = allocate_object_type(type_environment);
	obj->data.environment.bindings = bindings;
	return obj;
}

object* make_function(object* environment, object* parameters, object* body) {
	object* obj = allocate_object_type(type_function);
	obj->data.function.environment = environment;
	obj->data.function.parameters = parameters;
	obj->data.function.body = body;
	return obj;
}

object* make_primitive_procedure(object* parameters, primitive_proc proc) {
	object* obj = allocate_object_type(type_primitive_procedure);
	obj->data.primitive_procedure.parameters = parameters;
	obj->data.primitive_procedure.proc = proc;
	return obj;
}

int list_length(object* ls) {
	int n = 0;
	while (!is_empty_list(ls)) {
		n++;
		ls = list_rest(ls);
	}
	return n;
}

object* make_binding_list(object* names, object* values) {
	if (is_empty_list(names)) {
		return empty_list();
	}
	else {
		object* ls = allocate_list();
		object* prev;
		object* next = ls;
		while (!is_empty_list(names)) {
			prev = next;
			prev->data.list.first = make_binding(list_first(names), list_first(values));
		
			names = list_rest(names);
			values = list_rest(values);
		
			if (is_empty_list(names)) {
				next = empty_list();
			}
			else {
				next = allocate_list();
			}
			prev->data.list.rest = next;
		}
		return ls;
	}
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
	object* obj = allocate_object_type(type_number);
	obj->data.number.value = value;
	return obj;
}
