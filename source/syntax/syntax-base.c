#include "syntax-base.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "base-util.h"
#include "list-util.h"
#include "environments.h"
#include "call.h"
#include "eval.h"
#include "print.h"

object eval_syntax_proc;

object* eval_syntax(object* args, object* cont) {
	object* syntax;
	object* rest;
	object* environment;
	object* context;
	object* trace;
	delist_5(args, &syntax, &rest, &environment, &context, &trace);
	
	static_syntax_procedure syntax_id = syntax_procedure_id(syntax);
	context_type required_context = syntax_procedure_context(syntax_id);
	context_type current_context = eval_context_value(context);
	
	if (!(required_context & current_context)) {
		object* e = alloc_list_3(
			alloc_string(syntax_names[syntax_id]),
			alloc_string("not applicable in context"),
			alloc_string(context_names[current_context]));
		return throw_error(cont, e);
	}
	
	object* ls = alloc_list_3(rest, environment, trace);
	object* call = alloc_call(syntax, ls, cont);
	
	return perform_call(call);
}

object print_syntax_procedure_proc;

object* print_syntax_procedure(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	printf("syntax:%s", syntax_names[syntax_procedure_id(obj)]);
	
	return call_discarding_cont(cont);
}

object* quote(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* value;
	delist_desyntax_1(syntax, &value);
	
	return call_cont(cont, value);
}

object syntax_procedure[syntax_count];
context_type syntax_context[syntax_count];

context_type syntax_procedure_context(static_syntax_procedure type) {
	return syntax_context[type];
}

void set_syntax_properties(static_syntax_procedure syntax, char* name, context_type context) {
	syntax_names[syntax] = name;
	syntax_context[syntax] = context;
}

void init_syntax(static_syntax_procedure syntax, primitive_proc* proc) {
	init_syntax_procedure(&syntax_procedure[syntax], proc, syntax);
}

void add_syntax(char* name, static_syntax_procedure syntax, context_type context, primitive_proc* proc) {
	init_syntax(syntax, proc);
	set_syntax_properties(syntax, name, context);
	add_static_binding(name, &syntax_procedure[syntax]);
}

object* syntax_procedure_obj(static_syntax_procedure type) {
	return &syntax_procedure[type];
}

void init_base_syntax_procedures(void) {
	add_syntax("quote", syntax_quote, context_value, &quote);
	
	init_primitive(&eval_syntax, &eval_syntax_proc);
	add_list_application_procedure(type_syntax_procedure, &eval_syntax_proc);
	
	init_primitive(&print_syntax_procedure, &print_syntax_procedure_proc);
	add_print_procedure(type_syntax_procedure, &print_syntax_procedure_proc);
}
