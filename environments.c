#include "environments.h"
#include "global-variables.h"
#include "call.h"
#include "delist.h"

object* extend_environment(object* args, object* cont) {
	object* env;
	object* bindings;
	delist_2(args, &env, &bindings);
	if (!is_empty_list(bindings)) {
//		env = make_environment(list_append(bindings, environment_bindings(env)));
	}
	return call_cont(cont, env);
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
