#include "environments.h"
#include "list-util.h"
#include "global-variables.h"

object* extend_environment(object* env, object* bindings) {
	if (is_empty_list(bindings)) {
		return env;
	}
	else {
		return make_environment(list_append(bindings, environment_bindings(env)));
	}
}

object* find_in_environment(object* env, object* symbol) {
	object* ls = environment_bindings(env);
	while (!is_empty_list(ls)) {
		object* binding = list_first(ls);
		if (symbol == binding_name(binding)) {
			return binding;
		}
		ls = list_rest(ls);
	}
	return no_binding();
}