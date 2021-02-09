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
}
