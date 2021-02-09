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

object* add_static_binding(char* name, object* value) {
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
	init_list_cell(cell, binding, list_first(&_static_environment));
	make_static(cell);
	init_list_cell(&_static_environment, cell, list_rest(&_static_environment));
	make_static(&_static_environment);
	static_binding_count++;
	return binding;
}

object* extend_environment(object* args, object* cont) {
	object* binding;
	object* env;
	delist_2(args, &binding, &env);

	object* cell = alloc_list_cell(alloc_list_1(binding), env);
	
	return call_cont(cont, cell);
}

object* bind_and_extend_environment(object* args, object* cont) {
	object* value;
	object* name;
	object* env;
	delist_3(args, &value, &name, &env);
	
	object* binding = alloc_binding(desyntax(name), desyntax(value));
	env = alloc_list_cell(alloc_list_1(binding), env);
	
	return call_cont(cont, env);
}

object* bind_values(object* args, object* cont) {
	object* values;
	object* names;
	object* environment;
	delist_3(args, &values, &names, &environment);
	
	object* ls = empty_list();
	
	while (!is_empty_list(values)) {
		if (is_empty_list(names)) {
			return throw_error_string(cont, "arity mismatch, too many arguments");
		}
		object* binding = alloc_binding(desyntax(list_first(names)), desyntax(list_first(values)));
		ls = alloc_list_cell(binding, ls);
		values = list_rest(values);
		names = list_rest(names);
	}
	if (!is_empty_list(names)) {
		return throw_error_string(cont, "arity mismatch, too few arguments");
	}
	
	return call_cont(cont, alloc_list_cell(ls, environment));
}

object* environment_get(object* args, object* cont) {
	object* environment;
	object* name;
	delist_2(args, &environment, &name);
	
	object* binding = find_in_environment(environment, name, 1);
	if (is_no_binding(binding)) {
		object* str = alloc_string("not found in environment");
		return throw_error(cont, alloc_list_2(str, name));
	}
	return call_cont(cont, binding_value(binding));
}

object* find_in_environment(object* env, object* symbol, char return_placeholders) {
	while (!is_empty_list(env)) {
		object* ls = list_first(env);
		while (!is_empty_list(ls)) {
			object* binding = list_first(ls);
			if ((symbol == binding_name(binding)) && (return_placeholders || !is_placeholder_value(binding_value(binding)))) {
				return binding;
			}
			ls = list_rest(ls);
		}
		env = list_rest(env);
	}
	return no_binding();
}

void init_environment_procedures(void) {
	init_list_cell(&_static_environment, empty_list(), empty_list());
	make_static(&_static_environment);
	environment_init = 1;
	
	init_primitive(&extend_environment, &extend_environment_proc);
	init_primitive(&bind_and_extend_environment, &bind_and_extend_environment_proc);
	init_primitive(&bind_values, &bind_values_proc);
	init_primitive(&environment_get, &environment_get_proc);
}
