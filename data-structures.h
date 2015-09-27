#pragma once

typedef enum {
	type_none,
	type_boolean,
	type_symbol,
	type_number,
	type_list,
	type_function,
	type_primitive_procedure,
	type_binding,
	type_environment} object_type;

typedef enum {round, square, curly, shapeless, bracket_type_count} bracket_type;

typedef struct object* (*primitive_proc)(struct object* parameters);

typedef struct object {
	object_type type;
	union {
		struct {
			char value;
		} boolean;
		struct {
			char* name;
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
			struct object* parameters;
			struct object* environment;
			struct object* body;
		} function;
		struct {
			struct object* parameters;
			primitive_proc proc;
		} primitive_procedure;
		struct {
			struct object* name;
			struct object* value;
		} binding;
		struct {
			struct object* bindings;
		} environment;
	} data;
} object;

char list_start_delimiter[bracket_type_count] = {'(', '[', '{', '('};
char list_end_delimiter[bracket_type_count] = {')', ']', '}', ')'};

void check_type(object_type type, object* obj);

object* make_binding_list(object* names, object* values);
object* make_number(long value);
object* make_empty_list(bracket_type type);
object* make_binding(object* name, object* value);
object* make_environment(object* bindings);
object* make_function(object* environment, object* parameters, object* body);
object* make_primitive_procedure(object* parameters, primitive_proc proc);

char is_type(object_type type, object* obj);
char is_boolean(object* obj);
char is_symbol(object* obj);
char is_number(object* obj);
char is_list(object* obj);
char is_function(object* obj);
char is_primitive_procedure(object* obj);
char is_binding(object* obj);
char is_environment(object* obj);
char is_nonempty_list(object* obj);

char boolean_value(object* obj);
char* symbol_name(object* obj);
long number_value(object* obj);
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
object* primitive_procedure_parameters(object* obj);
primitive_proc primitive_procedure_proc(object* obj);
object* binding_name(object* obj);
object* binding_value(object* obj);
object* environment_bindings(object* obj);
object* add_symbol(char* name);
object* find_symbol(char* name);

int list_length(object* ls);
object* extend_environment(object* env, object* bindings);
object* find_in_environment(object* env, object* symbol);
char is_self_quoting(object* obj);
object* quote(object* value);
char list_starts_with(object* ls, object* obj);
char is_quoted(object* exp);
char is_definition(object* exp);
