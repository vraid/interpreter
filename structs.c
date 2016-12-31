#include "structs.h"

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
#include "environments.h"
#include "base-syntax.h"
#include "standard-library.h"
#include "generic-arguments.h"
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

object add_struct_constructor_next_proc;

object* add_struct_constructor_next(object* args, object* cont) {
	object* fields;
	object* type;
	delist_2(args, &fields, &type);
	
	object field_list;
	init_list_cell(&field_list, syntax_obj(syntax_list), fields);
	object vector_list[2];
	init_list_2(vector_list, &list_to_vector_proc, &field_list);
	object body[3];
	init_list_3(body, &make_struct_proc, type, vector_list);
	object function;
	init_function(&function, empty_environment(), fields, body);
	
	type->data.struct_definition.fields = fields;
	add_mutation(type, fields);
	type->data.struct_definition.constructor = &function;
	add_mutation(type, &function);
	
	return call_cont(cont, type);
}

object add_struct_constructor_proc;

object* add_struct_constructor(object* args, object* cont) {
	object* fields;
	object* type;
	delist_2(args, &fields, &type);
	
	object next_args[1];
	init_list_1(next_args, type);
	object next_call;
	init_call(&next_call, &add_struct_constructor_next_proc, next_args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	
	object append_list[2];
	init_list_2(append_list, struct_definition_fields(struct_definition_parent(type)), fields);
	object append_args[1];
	init_list_1(append_args, append_list);
	object append_call;
	init_call(&append_call, &list_append_proc, append_args, &next_cont);
	
	return perform_call(&append_call);
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
		init_fixnum(&next_count, fixnum_value(count)+1);
		
		object next_args[3];
		init_list_3(next_args, list_rest(fields), &next_count, type);
		object next_call;
		init_call(&next_call, &define_field_accessors_proc, next_args, cont);
		object next_cont;
		init_cont(&next_cont, &next_call);
		
		object field_args[4];
		init_list_4(field_args, first, count, type, environment);
		object field_call;
		init_call(&field_call, &define_field_accessor_proc, field_args, &next_cont);
		
		return perform_call(&field_call);
	}
}

object define_struct_next_proc;

object* define_struct_next(object* args, object* cont) {
	object* renamed_fields;
	object* type;
	object* environment;
	delist_3(args, &renamed_fields, &type, &environment);
	
	object access_start;
	init_fixnum(&access_start, list_length(struct_definition_fields(struct_definition_parent(type))));
	
	object field_args[3];
	init_list_3(field_args, renamed_fields, &access_start, type);
	object field_call;
	init_call(&field_call, &define_field_accessors_proc, field_args, cont);
	object field_cont;
	init_cont(&field_cont, &field_call);
	
	object type_args[1];
	init_list_1(type_args, type);
	object type_call;
	init_call(&type_call, &define_struct_is_type_proc, type_args, &field_cont);
	object type_cont;
	init_cont(&type_cont, &type_call);
	
	object bind_args[2];
	init_list_2(bind_args, struct_definition_name(type), environment);
	object bind_call;
	init_call(&bind_call, &extend_environment_proc, bind_args, &type_cont);
	object bind_cont;
	init_cont(&bind_cont, &bind_call);
	
	object constructor_args[2];
	init_list_2(constructor_args, renamed_fields, type);
	object constructor_call;
	init_call(&constructor_call, &add_struct_constructor_proc, constructor_args, &bind_cont);
	
	return perform_call(&constructor_call);
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
		delist_2(syntax, &name, &fields);
		parent = empty_struct_definition();
	}
	else if (length == 3) {
		delist_3(syntax, &name, &parent, &fields);
		parent = binding_value(find_in_environment(environment, parent, 0));
	}
	else {
		return throw_error(cont, "invalid struct definition");
	}
	
	object struct_type;
	init_struct_definition(&struct_type, name, empty_list(), no_object(), parent);
	
	object next_args[2];
	init_list_2(next_args, &struct_type, environment);
	object next_call;
	init_call(&next_call, &define_struct_next_proc, next_args, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
		
	object append_args[4];
	init_list_4(append_args, syntax_obj(syntax_list), symbol_name(name), dash_string(), generic_args[0]);
	object append_list[2];
	init_list_2(append_list, &string_append_proc, append_args);
	object symbol_list[2];
	init_list_2(symbol_list, &string_to_symbol_proc, append_list);
	object function[3];
	init_list_3(function, syntax_obj(syntax_lambda), generic_arg_list[1], symbol_list);
	object quote[2];
	init_list_2(quote, syntax_obj(syntax_quote), fields);
	object field_names[3];
	init_list_3(field_names, syntax_obj(syntax_map), symbol_to_string_func, quote);
	object map_list[3];
	init_list_3(map_list, syntax_obj(syntax_map), function, field_names);
	object eval_args[2];
	init_list_2(eval_args, map_list, empty_environment());
	object eval_call;
	init_call(&eval_call, &eval_proc, eval_args, &next_cont);
	
	return perform_call(&eval_call);
}

void init_struct_procedures(void) {
	init_primitive(&make_struct, &make_struct_proc);
	init_primitive(&add_struct_constructor, &add_struct_constructor_proc);
	init_primitive(&add_struct_constructor_next, &add_struct_constructor_next_proc);
	init_primitive(&struct_access, &struct_access_proc);
	init_primitive(&define_field_accessors, &define_field_accessors_proc);
	init_primitive(&define_field_accessor, &define_field_accessor_proc);
	init_primitive(&make_field_accessor, &make_field_accessor_proc);
	init_primitive(&struct_is_type_func, &struct_is_type_proc);
	init_primitive(&bind_struct_is_type, &bind_struct_is_type_proc);
	init_primitive(&define_struct_is_type, &define_struct_is_type_proc);
	init_primitive(&define_struct, &define_struct_proc);
	init_primitive(&define_struct_next, &define_struct_next_proc);
}
