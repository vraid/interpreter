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

object* add_static_binding(char* name, object* value) {
	if (static_binding_count == static_binding_max) {
		fprintf(stderr, "too many static bindings\n");
		exit(0);
	}
	object* symbol = make_static_symbol(name);
	object* binding = &static_bindings[static_binding_count];
	object* cell = &static_binding_cell[static_binding_count];
	init_binding(binding, symbol, value);
	binding->location = location_static;
	init_list_cell(cell, binding, static_environment()->data.environment.bindings);
	cell->location = location_static;
	static_environment()->data.environment.bindings = cell;
	static_binding_count++;
	return binding;
}

object* extend_environment(object* args, object* cont) {
	object* name;
	object* value;
	object* env;
	delist_3(args, &name, &value, &env);
	
	object binding;
	init_binding(&binding, name, value);
	object cell;
	init_list_cell(&cell, binding, environment_bindings(env));
	object new_env;
	init_environment(&new_env, &cell);
	
	return call_cont(cont, &new_env);
}

object* find_in_environment(object* env, object* symbol) {
	object* ls = environment_bindings(env);
	while (!is_empty_list(ls)) {
		object* binding = list_first(ls);
		if (symbol == binding_name(binding)) {
			return binding_value(binding);
		}
		ls = list_rest(ls);
	}
	return no_object();
}

void init_environment_procedures(void) {
	init_environment(&_static_environment, empty_list());
	_static_environment.location = location_static;
	
	init_primitive_procedure(&extend_environment_proc, &extend_environment);
}
