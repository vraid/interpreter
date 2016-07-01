#include "structs.h"

#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"
#include "vector-util.h"
#include "strings.h"
#include "symbols.h"
#include "environments.h"
#include "base-syntax.h"
#include "standard-library.h"
#include "eval.h"

object make_struct_proc;

object* make_struct(object* args, object* cont) {
	object* type;
	object* data;
	delist_2(args, &type, &data);
	
	object instance;
	init_struct_instance(&instance, type, data);
	
	return call_cont(cont, &instance);
}

object define_struct_constructor_proc;

object* define_struct_constructor(object* args, object* cont) {
	object* name;
	object* fields;
	object* type;
	object* environment;
	delist_4(args, &name, &fields, &type, &environment);
	
	object vector_list;
	init_list_cell(&vector_list, syntax_obj(syntax_vector), fields);
	object body[3];
	init_list_3(body, &make_struct_proc, type, &vector_list);
	object function;
	init_function(&function, empty_environment(), fields, body);
	
	object bind_args[3];
	init_list_3(bind_args, &function, name, environment);
	object bind_call;
	init_call(&bind_call, &extend_environment_proc, bind_args, cont);
	
	return perform_call(&bind_call);
}

char struct_is_type(object* type, object* st) {
	return is_struct_instance(st) && (type == struct_instance_type(st));
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
	
	object bind_args[3];
	init_list_3(bind_args, value, name, environment);
	object bind_call;
	init_call(&bind_call, &extend_environment_proc, bind_args, cont);
	
	return perform_call(&bind_call);
}

object define_struct_is_type_proc;

object* define_struct_is_type(object* args, object* cont) {
	object* environment;
	object* type;
	delist_2(args, &environment, &type);
	
	object* name = struct_definition_name(type);
	
	object body[3];
	init_list_3(body, &struct_is_type_proc, type, generic_args[0]);
	object function;
	init_function(&function, empty_environment(), generic_arg_list[1], body);
	
	object bind_args[2];
	init_list_2(bind_args, &function, environment);
	object bind_call;
	init_call(&bind_call, &bind_struct_is_type_proc, bind_args, cont);
	object bind_cont;
	init_cont(&bind_cont, &bind_call);
	
	object symbol_call;
	init_call(&symbol_call, &string_to_symbol_proc, empty_list(), &bind_cont);
	object symbol_cont;
	init_cont(&symbol_cont, &symbol_call);
	
	object name_list[2];
	init_list_2(name_list, symbol_name(name), question_mark_string());
	object name_args[1];
	init_list_1(name_args, name_list);
	object name_call;
	init_call(&name_call, &string_append_proc, name_args, &symbol_cont);
	
	return perform_call(&name_call);
}

object struct_access_proc;

object* struct_access(object* args, object* cont) {
	object* type;
	object* num;
	object* st;
	delist_3(args, &type, &num, &st);
	
	if (!struct_is_type(type, st)) {
		return throw_error(cont, "incorrect struct type");
	}
	else {
		object* data = struct_instance_data(st);
		int n = number_value(num);
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
	object arg_list[1];
	init_list_1(arg_list, arg);
	
	object body[4];
	init_list_4(body, &struct_access_proc, type, count, arg);
	object function;
	init_function(&function, empty_environment(), arg_list, body);
	
	return call_cont(cont, &function);
}

object define_field_accessor_proc;

object* define_field_accessor(object* args, object* cont) {
	object* name;
	object* count;
	object* type;
	object* environment;
	delist_4(args, &name, &count, &type, &environment);
	
	object bind_args[2];
	init_list_2(bind_args, name, environment);
	object bind_call;
	init_call(&bind_call, &extend_environment_proc, bind_args, cont);
	object bind_cont;
	init_cont(&bind_cont, &bind_call);
	
	object make_args[2];
	init_list_2(make_args, count, type);
	object make_call;
	init_call(&make_call, &make_field_accessor_proc, make_args, &bind_cont);
	
	return perform_call(&make_call);
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
		
		object next_count;
		init_number(&next_count, number_value(count)+1);
		
		object next_args[3];
		init_list_3(next_args, list_rest(fields), &next_count, type);
		object next_call;
		init_call(&next_call, &define_field_accessors_proc, next_args, cont);
		object next_cont;
		init_cont(&next_cont, &next_call);
		
		object field_args[3];
		init_list_3(field_args, count, type, environment);
		object field_call;
		init_call(&field_call, &define_field_accessor_proc, field_args, &next_cont);
		object field_cont;
		init_cont(&field_cont, &field_call);
		
		object symbol_call;
		init_call(&symbol_call, &string_to_symbol_proc, empty_list(), &field_cont);
		object symbol_cont;
		init_cont(&symbol_cont, &symbol_call);
		
		object name_list[3];
		init_list_3(name_list, symbol_name(struct_definition_name(type)), dash_string(), symbol_name(first));
		object name_args[1];
		init_list_1(name_args, name_list);
		object name_call;
		init_call(&name_call, &string_append_proc, name_args, &symbol_cont);
		
		return perform_call(&name_call);
	}
}

object* define_struct(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* name;
	object* fields;
	delist_2(syntax, &name, &fields);
	
	object struct_type;
	init_struct_definition(&struct_type, name, fields);
	
	object field_args[3];
	init_list_3(field_args, fields, zero(), &struct_type);
	object field_call;
	init_call(&field_call, &define_field_accessors_proc, field_args, cont);
	object field_cont;
	init_cont(&field_cont, &field_call);
	
	object type_args[1];
	init_list_1(type_args, &struct_type);
	object type_call;
	init_call(&type_call, &define_struct_is_type_proc, type_args, &field_cont);
	object type_cont;
	init_cont(&type_cont, &type_call);
	
	object constructor_args[4];
	init_list_4(constructor_args, name, fields, &struct_type, environment);
	object constructor_call;
	init_call(&constructor_call, &define_struct_constructor_proc, constructor_args, &type_cont);
	
	return perform_call(&constructor_call);
}

void init_struct_procedures(void) {
	init_primitive_procedure(&make_struct_proc, &make_struct);
	init_primitive_procedure(&define_struct_constructor_proc, &define_struct_constructor);
	init_primitive_procedure(&struct_access_proc, &struct_access);
	init_primitive_procedure(&define_field_accessors_proc, &define_field_accessors);
	init_primitive_procedure(&define_field_accessor_proc, &define_field_accessor);
	init_primitive_procedure(&make_field_accessor_proc, &make_field_accessor);
	init_primitive_procedure(&struct_is_type_proc, &struct_is_type_func);
	init_primitive_procedure(&bind_struct_is_type_proc, &bind_struct_is_type);
	init_primitive_procedure(&define_struct_is_type_proc, &define_struct_is_type);
	init_primitive_procedure(&define_struct_proc, &define_struct);
}
