#include "syntax-validate.h"

#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "call.h"

object* no_validate(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	return call_cont(cont, stx);
}

primitive_proc* syntax_validate[syntax_count];

object* validate_syntax(object* args, object* cont) {
	object* stx;
	object* env;
	delist_2(args, &stx, &env);
	
	object proc;
	init_primitive_procedure(&proc, syntax_validate[syntax_id(list_first(stx))]);
	object call;
	init_call(&call, &proc, args, cont);
	
	return perform_call(&call);
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
		return call_cont(cont, stx);
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
		syntax_validate[i] = &no_validate;
	}
	
	init_primitive(&validate_expression, &validate_expression_proc);
	init_primitive(&validate_list, &validate_list_proc);
}
