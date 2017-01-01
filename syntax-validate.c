#include "syntax-validate.h"

#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"
#include "environments.h"
#include "higher-order.h"
#include "generic-arguments.h"

object* no_validate(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	return call_cont(cont, stx);
}

object syntax_validate[syntax_count];

object validate_list_elements_proc;

object* validate_list_elements(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object body[3];
	init_list_3(body, &validate_expression_proc, generic_args[0], env);
	object func;
	init_function(&func, empty_environment(), generic_arg_list[1], body);
	
	object map_list[2];
	init_list_2(map_list, &func, stx);
	object map_args[1];
	init_list_1(map_args, map_list);
	object map_call;
	init_call(&map_call, &map_proc, map_args, cont);
	
	return perform_call(&map_call);
}

object validate_list_proc;

object* validate_list(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	if (is_empty_list(stx)) {
		return throw_error(cont, "expression cannot be empty list");
	}
	else {
		object* obj = list_first(stx);
		if (is_symbol(obj)) {
			object* a = find_in_environment(env, obj, 1);
			if (is_syntax(a)) {
				object call;
				init_call(&call, &syntax_validate[syntax_id(a)], args, cont);
				return perform_call(&call);
			}
		}
		object list_call;
		init_call(&list_call, &validate_list_elements_proc, args, cont);
		return perform_call(&list_call);
	}
}

object* validate_expression(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	if (is_list(stx)) {
		object call;
		init_call(&call, &validate_list_proc, args, cont);
		return perform_call(&call);
	}
	else {
		return call_cont(cont, stx);
	}
}

void init_validate_procedures(void) {
	int i;
	for (i = 0; i < syntax_count; i++) {
		init_primitive_procedure(&syntax_validate[i], &no_validate);
	}
	
	init_primitive(&validate_expression, &validate_expression_proc);
	init_primitive(&validate_list, &validate_list_proc);
	init_primitive(&validate_list_elements, &validate_list_elements_proc);
}
