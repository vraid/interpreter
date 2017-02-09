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

object* static_environment(void) {
	return &_static_environment;
}

object* add_static_binding(object* value, char* name) {
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
	object* value;
	object* name;
	object* env;
	delist_3(args, &value, &name, &env);
	
	object binding;
	init_binding(&binding, name, desyntax(value));
	object cell;
	init_list_cell(&cell, &binding, environment_bindings(env));
	object new_env;
	init_environment(&new_env, &cell);
	
	return call_cont(cont, &new_env);
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
		object next_ls[2];
		init_list_2(next_ls, list_rest(values), list_rest(names));
		object next_call;
		init_call(&next_call, &bind_single_value_proc, next_ls, cont);
		object next_cont;
		init_cont(&next_cont, &next_call);
		
		object bind_ls[3];
		init_list_3(bind_ls, list_first(values), desyntax(list_first(names)), environment);
		object bind_call;
		init_call(&bind_call, &extend_environment_proc, bind_ls, &next_cont);
		
		return perform_call(&bind_call);
	}
}

object* bind_values(object* args, object* cont) {
	object* values;
	object* names;
	object* environment;
	delist_3(args, &values, &names, &environment);
	
	object ls[3];
	init_list_3(ls, environment, values, names);
	
	object call;
	init_call(&call, &bind_single_value_proc, ls, cont);
	
	return perform_call(&call);
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
	make_static(&_static_environment);
	
	init_primitive(&extend_environment, &extend_environment_proc);
	init_primitive(&bind_values, &bind_values_proc);
	init_primitive(&bind_single_value, &bind_single_value_proc);
}
