#include "structs.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "memory-handling.h"
#include "call.h"
#include "vectors.h"
#include "strings.h"
#include "symbols.h"
#include "list-util.h"
#include "string-util.h"
#include "environments.h"
#include "base-syntax.h"
#include "standard-library.h"
#include "generic-arguments.h"
#include "eval.h"

object make_struct_instance_proc;

object* make_struct_instance(object* args, object* cont) {
	object* type;
	object* data;
	delist_2(args, &type, &data);
	
	object* instance = alloc_struct_instance(type, data);
	
	return call_cont(cont, instance);
}

object add_struct_constructor_next_proc;

object* add_struct_constructor_next(object* args, object* cont) {
	object* fields;
	object* type;
	delist_2(args, &fields, &type);
	
	object* vector_list = alloc_list_cell(syntax_procedure_obj(syntax_vector), fields);
	object* body = alloc_list_3(&make_struct_instance_proc, type, vector_list);
	object* function = alloc_function(empty_environment(), fields, body);
	
	type->data.struct_definition.fields = fields;
	alloc_stack_reference(type, fields);
	type->data.struct_definition.constructor = function;
	alloc_stack_reference(type, function);
	
	return call_cont(cont, type);
}

object add_struct_constructor_proc;

object* add_struct_constructor(object* args, object* cont) {
	object* fields;
	object* type;
	delist_2(args, &fields, &type);
	
	object* next_args = alloc_list_1(type);
	object* next_call = alloc_call(&add_struct_constructor_next_proc, next_args, cont);
	object* next_cont = alloc_cont(next_call);
	
	object* append_list = alloc_list_2(struct_definition_fields(struct_definition_parent(type)), fields);
	object* append_args = alloc_list_1(append_list);
	object* append_call = alloc_call(&list_append_proc, append_args, next_cont);
	
	return perform_call(append_call);
}

char struct_is_type(object* type, object* st) {
	if (is_struct_instance(st)) {
		object* t = struct_instance_type(st);
		while (!is_empty_struct_definition(t)) {
			if (t == type) return 1;
			t = struct_definition_parent(t);	
		}
	}
	return 0;
}

object struct_is_type_proc;

object* struct_is_type_func(object* args, object* cont) {
	object* type;
	object* st;
	delist_2(args, &type, &st);
	
	return call_cont(cont, boolean(struct_is_type(type, st)));
}

object bind_struct_is_type_proc;

object* bind_struct_is_type(object* args, object* cont) {
	object* name;
	object* value;
	object* environment;
	delist_3(args, &name, &value, &environment);
	
	object* bind_args = alloc_list_3(value, name, environment);
	object* bind_call = alloc_call(&extend_environment_proc, bind_args, cont);
	
	return perform_call(bind_call);
}

object define_struct_is_type_proc;

object* define_struct_is_type(object* args, object* cont) {
	object* environment;
	object* type;
	delist_2(args, &environment, &type);
	
	object* name = struct_definition_name(type);
	
	object* body = alloc_list_3(&struct_is_type_proc, type, generic_args[0]);
	object* function = alloc_function(empty_environment(), generic_arg_list[1], body);
	
	object* bind_args = alloc_list_2(function, environment);
	object* bind_call = alloc_call(&bind_struct_is_type_proc, bind_args, cont);
	object* bind_cont = alloc_cont(bind_call);
	
	object* is_name = alloc_append_string_2(symbol_name(name), question_mark_string());
	object* symbol_args = alloc_list_1(is_name);
	object* symbol_call = alloc_call(&string_to_symbol_proc, symbol_args, bind_cont);
	
	return perform_call(symbol_call);
}

object struct_access_proc;

object* struct_access(object* args, object* cont) {
	object* type;
	object* num;
	object* st;
	delist_3(args, &type, &num, &st);
	
	if (!struct_is_type(type, st)) {
		return throw_error_string(cont, "incorrect struct type");
	}
	else {
		object* data = struct_instance_data(st);
		int n = fixnum_value(num);
		object* element = vector_ref(data, n);
		
		return call_cont(cont, element);
	}
}

object make_field_accessor_proc;

object* make_field_accessor(object* args, object* cont) {
	object* count;
	object* type;
	delist_2(args, &count, &type);
	
	object* arg = struct_definition_name(type);
	object* arg_list = alloc_list_1(arg);
	
	object* body = alloc_list_4(&struct_access_proc, type, count, arg);
	object* function = alloc_function(empty_environment(), arg_list, body);
	
	return call_cont(cont, function);
}

object define_field_accessor_proc;

object* define_field_accessor(object* args, object* cont) {
	object* name;
	object* count;
	object* type;
	object* environment;
	delist_4(args, &name, &count, &type, &environment);
	
	object* bind_args = alloc_list_2(name, environment);
	object* bind_call = alloc_call(&extend_environment_proc, bind_args, cont);
	object* bind_cont = alloc_cont(bind_call);
	
	object* make_args = alloc_list_2(count, type);
	object* make_call = alloc_call(&make_field_accessor_proc, make_args, bind_cont);
	
	return perform_call(make_call);
}

object define_field_accessors_proc;

object* define_field_accessors(object* args, object* cont) {
	object* environment;
	object* fields;
	object* count;
	object* type;
	delist_4(args, &environment, &fields, &count, &type);
	
	if (is_empty_list(fields)) {
		return call_cont(cont, environment);
	}
	else {
		object* first = list_first(fields);
		
		object* next_count = alloc_fixnum(fixnum_value(count)+1);
		
		object* next_args = alloc_list_3(list_rest(fields), next_count, type);
		object* next_call = alloc_call(&define_field_accessors_proc, next_args, cont);
		object* next_cont = alloc_cont(next_call);
		
		object* field_args = alloc_list_4(first, count, type, environment);
		object* field_call = alloc_call(&define_field_accessor_proc, field_args, next_cont);
		
		return perform_call(field_call);
	}
}

object define_struct_next_proc;

object* define_struct_next(object* args, object* cont) {
	object* renamed_fields;
	object* type;
	object* environment;
	delist_3(args, &renamed_fields, &type, &environment);
	
	object* access_start = alloc_fixnum(list_length(struct_definition_fields(struct_definition_parent(type))));
	
	object* field_args = alloc_list_3(renamed_fields, access_start, type);
	object* field_call = alloc_call(&define_field_accessors_proc, field_args, cont);
	object* field_cont = alloc_cont(field_call);
	
	object* type_args = alloc_list_1(type);
	object* type_call = alloc_call(&define_struct_is_type_proc, type_args, field_cont);
	object* type_cont = alloc_cont(type_call);
	
	object* bind_args = alloc_list_2(struct_definition_name(type), environment);
	object* bind_call = alloc_call(&extend_environment_proc, bind_args, type_cont);
	object* bind_cont = alloc_cont(bind_call);
	
	object* constructor_args = alloc_list_2(renamed_fields, type);
	object* constructor_call = alloc_call(&add_struct_constructor_proc, constructor_args, bind_cont);
	
	return perform_call(constructor_call);
}

object struct_field_names_proc;

object* struct_field_names(object* args, object* cont) {
	object* result;
	object* fields;
	object* name;
	delist_3(args, &result, &fields, &name);
	
	if (is_empty_list(fields)) {
		object* call_args = alloc_list_1(result);
		object* call = alloc_call(&reverse_list_proc, call_args, cont);
		
		return perform_call(call);
	}
	else {
		object* next_args = alloc_list_2(list_rest(fields), name);
		object* next_call = alloc_call(&struct_field_names_proc, next_args, cont);
		object* next_cont = alloc_cont(next_call);
		
		object* link_args = alloc_list_1(result);
		object* link_call = alloc_call(&link_list_proc, link_args, next_cont);
		object* link_cont = alloc_cont(link_call);
		
		object* fieldname = alloc_append_string_3(name, dash_string(), symbol_name(desyntax(list_first(fields))));
		object* symbol_args = alloc_list_1(fieldname);
		object* symbol_call = alloc_call(&string_to_symbol_proc, symbol_args, link_cont);
		
		return perform_call(symbol_call);
	}
}

object* define_struct(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	int length = list_length(syntax);
	
	object* name;
	object* fields;
	object* parent;
	
	if (length == 2) {
		delist_desyntax_2(syntax, &name, &fields);
		parent = empty_struct_definition();
	}
	else if (length == 3) {
		delist_desyntax_3(syntax, &name, &parent, &fields);
		parent = binding_value(find_in_environment(environment, parent, 0));
	}
	else {
		object* str = alloc_string("invalid struct definition");
		object* ls = alloc_list_2(str, syntax);
		return throw_error(cont, ls);
	}
	
	object* struct_type = alloc_struct_definition(name, empty_list(), no_object(), parent);
	
	object* next_args = alloc_list_2(struct_type, environment);
	object* next_call = alloc_call(&define_struct_next_proc, next_args, cont);
	object* next_cont = alloc_cont(next_call);
	
	object* name_args = alloc_list_3(empty_list(), fields, symbol_name(name));
	object* name_call = alloc_call(&struct_field_names_proc, name_args, next_cont);
	
	return perform_call(name_call);
}

void init_struct_procedures(void) {
	init_primitive(&make_struct_instance, &make_struct_instance_proc);
	init_primitive(&add_struct_constructor, &add_struct_constructor_proc);
	init_primitive(&add_struct_constructor_next, &add_struct_constructor_next_proc);
	init_primitive(&struct_access, &struct_access_proc);
	init_primitive(&define_field_accessors, &define_field_accessors_proc);
	init_primitive(&define_field_accessor, &define_field_accessor_proc);
	init_primitive(&make_field_accessor, &make_field_accessor_proc);
	init_primitive(&struct_is_type_func, &struct_is_type_proc);
	init_primitive(&bind_struct_is_type, &bind_struct_is_type_proc);
	init_primitive(&define_struct_is_type, &define_struct_is_type_proc);
	init_primitive(&struct_field_names, &struct_field_names_proc);
	init_primitive(&define_struct, &define_struct_proc);
	init_primitive(&define_struct_next, &define_struct_next_proc);
}
