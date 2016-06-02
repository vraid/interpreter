#pragma once

#include <stdio.h>

typedef enum {
	type_none,
	type_boolean,
	type_string,
	type_symbol,
	type_number,
	type_list,
	type_primitive_procedure,
	type_function,
	type_call,
	type_continuation,
	type_binding,
	type_environment,
	type_file_port,
	type_internal_error,
	type_count} object_type;

char* type_name[type_count];

typedef enum {
	location_none,
	location_stack,
	location_heap,
	location_static,
	location_moved,
	location_count} object_location;

char* location_name[location_count];

typedef enum {round, square, curly, shapeless, file_bracket, bracket_type_count} bracket_type;

typedef struct object* (primitive_proc)(struct object* args, struct object* cont);

typedef struct object {
	object_location location;
	object_type type;
	union {
		struct {
			struct object* ref;
		} forward_reference;
		struct {
			char value;
		} boolean;
		struct {
			int length;
			char* value;
		} string;
		struct {
			struct object* name;
		} symbol;
		struct {
			long value;
		} number;
		struct {
			bracket_type type;
			struct object* first;
			struct object* rest;
		} list;
		struct {
			primitive_proc* proc;
		} primitive_procedure;
		struct {
			struct object* parameters;
			struct object* environment;
			struct object* body;
		} function;
		struct {
			struct object* function;
			struct object* arguments;
			struct object* continuation;
		} call;
		struct {
			char discard_argument;
			struct object* call;
		} continuation;
		struct {
			struct object* name;
			struct object* value;
		} binding;
		struct {
			struct object* bindings;
		} environment;
		struct {
			FILE* file;
		} file_port;
		struct {
			struct object* message;
		} internal_error;
	} data;
} object;

void init_data_structure_names(void);

char list_start_delimiter[bracket_type_count];
char list_end_delimiter[bracket_type_count];

char* object_type_name(object* obj);
void check_type(object_type type, object* obj);

char is_type(object_type type, object* obj);
char is_boolean(object* obj);
char is_string(object* obj);
char is_symbol(object* obj);
char is_number(object* obj);
char is_list(object* obj);
char is_function(object* obj);
char is_primitive_procedure(object* obj);
char is_binding(object* obj);
char is_environment(object* obj);
char is_file_port(object* obj);
char is_call(object* obj);
char is_nonempty_list(object* obj);
char is_continuation(object* obj);
char is_discarding_continuation(object* obj);
char is_internal_error(object* obj);

char boolean_value(object* obj);
int string_length(object* obj);
char* string_value(object* obj);
object* symbol_name(object* obj);
long number_value(object* obj);
FILE* file_port_file(object* obj);
bracket_type list_type(object* obj);
object* list_first(object* ls);
object* list_rest(object* ls);
object* cons(object* first, object* rest);
object* list_ref(int n, object* ls);
object* list_take(int n, object* obj);
object* list_drop(int n, object* obj);
object* append(object* as, object* rest);
object* reverse(object* obj);
object* function_parameters(object* obj);
object* function_environment(object* obj);
object* function_body(object* obj);
primitive_proc* primitive_procedure_proc(object* obj);
object* binding_name(object* obj);
object* binding_value(object* obj);
object* environment_bindings(object* obj);
object* call_function(object* obj);
object* call_arguments(object* obj);
object* call_continuation(object* obj);
object* continuation_call(object* obj);
object* internal_error_message(object* obj);

int list_length(object* ls);
object* extend_environment(object* env, object* bindings);
object* find_in_environment(object* env, object* symbol);
char is_self_quoting(object* obj);
object* quote(object* value);
char list_starts_with(object* ls, object* obj);
char is_quoted(object* exp);
char is_definition(object* exp);
