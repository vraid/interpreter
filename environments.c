#include "environments.h"

#include <stdlib.h>
#include "global-variables.h"
#include "symbols.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"

#define static_binding_max 1024
object _static_environment;
object static_bindings[static_binding_max];
object static_binding_cell[static_binding_max];
int static_binding_count = 0;
char environment_init = 0;

object* static_environment(void) {
	return &_static_environment;
}

object* add_static_binding(object* value, char* name) {
	if (!environment_init) {
		fprintf(stderr, "environment not initialized\n");
		exit(0);
	}	
	if (static_binding_count == static_binding_max) {
		fprintf(stderr, "too many static bindings\n");
		exit(0);
	}
	object* symbol = make_static_symbol(name);
	object* binding = &static_bindings[static_binding_count];
	object* cell = &static_binding_cell[static_binding_count];
	init_binding(binding, symbol, value);
	make_static(binding);
	init_list_cell(cell, binding, static_environment()->data.environment.bindings);
	make_static(cell);
	static_environment()->data.environment.bindings = cell;
	static_binding_count++;
	return binding;
}

object* extend_environment(object* args, object* cont) {
	object* binding;
	object* env;
	delist_2(args, &binding, &env);

	object* cell = alloc_list_cell(binding, environment_bindings(env));
	object* new_env = alloc_environment(cell);
	
	return call_cont(cont, new_env);
}

object make_binding_proc;

object* make_binding(object* args, object* cont) {
	object* value;
	object* name;
	delist_2(args, &value, &name);
	
	object* binding = alloc_binding(desyntax(name), desyntax(value));
	return call_cont(cont, binding);
}

object* bind_and_extend_environment(object* args, object* cont) {
	object* value;
	object* name;
	object* env;
	delist_3(args, &value, &name, &env);
	
	object* bind_args = alloc_list_1(env);
	object* bind_call = alloc_call(&extend_environment_proc, bind_args, cont);
	object* bind_cont = alloc_cont(bind_call);
	
	object* make_args = alloc_list_2(value, name);
	object* make_call = alloc_call(&make_binding_proc, make_args, bind_cont);
	
	return perform_call(make_call);
}

object bind_single_value_proc;

object* bind_single_value(object* args, object* cont) {
	object* environment;
	object* values;
	object* names;
	delist_3(args, &environment, &values, &names);
	
	if (is_empty_list(values)) {
		return call_cont(cont, environment);
	}
	else if (is_empty_list(names)) {
		return throw_error_string(cont, "arity mismatch, too many arguments");
	}
	else {
		object* next_ls = alloc_list_2(list_rest(values), list_rest(names));
		object* next_call = alloc_call(&bind_single_value_proc, next_ls, cont);
		object* next_cont = alloc_cont(next_call);
		
		object* bind_ls = alloc_list_1(environment);
		object* bind_call = alloc_call(&extend_environment_proc, bind_ls, next_cont);
		object* bind_cont = alloc_cont(bind_call);
		
		object* make_binding_args = alloc_list_2(list_first(values), list_first(names));
		object* make_binding_call = alloc_call(&make_binding_proc, make_binding_args, bind_cont);
		
		return perform_call(make_binding_call);
	}
}

object* bind_values(object* args, object* cont) {
	object* values;
	object* names;
	object* environment;
	delist_3(args, &values, &names, &environment);
	
	object* ls = alloc_list_3(environment, values, names);
	object* call = alloc_call(&bind_single_value_proc, ls, cont);
	
	return perform_call(call);
}

object* find_in_environment(object* env, object* symbol, char return_placeholders) {
	object* ls = environment_bindings(env);
	while (!is_empty_list(ls)) {
		object* binding = list_first(ls);
		if ((symbol == binding_name(binding)) && (return_placeholders || !is_placeholder_value(binding_value(binding)))) {
			return binding;
		}
		ls = list_rest(ls);
	}
	return no_binding();
}

void init_environment_procedures(void) {
	init_environment(&_static_environment, empty_list());
	environment_init = 1;
	make_static(&_static_environment);
	
	init_primitive(&extend_environment, &extend_environment_proc);
	init_primitive(&bind_and_extend_environment, &bind_and_extend_environment_proc);
	init_primitive(&make_binding, &make_binding_proc);
	init_primitive(&bind_values, &bind_values_proc);
	init_primitive(&bind_single_value, &bind_single_value_proc);
}
