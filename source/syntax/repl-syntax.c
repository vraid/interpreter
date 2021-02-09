#include "repl-syntax.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "base-util.h"
#include "list-util.h"
#include "memory-handling.h"
#include "syntax-base.h"
#include "call.h"
#include "repl-top.h"

object* enter_scope(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* key;
	object* read_table;
	delist_2(syntax, &key, &read_table);
	
	object* call = alloc_call(&repl_read_entry_proc, alloc_list_2(read_table, environment), cont);
	alloc_repl_scope_reference(alloc_list_2(key, call));
	
	return perform_call(call);
}

object* rewind_scope(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* key;
	delist_1(syntax, &key);
	
	object* call = rewound_repl_scope_reference(key);
	if (is_no_object(call)) {
		return throw_error(cont, alloc_list_2(alloc_string("could not rewind scope to key"), key));
	}
	return perform_call(call);
}

void init_repl_syntax_procedures(void) {
	init_syntax(syntax_enter_scope, &enter_scope);
	set_syntax_properties(syntax_enter_scope, "enter-scope", context_scope);
	init_syntax(syntax_rewind_scope, &rewind_scope);
	set_syntax_properties(syntax_rewind_scope, "rewind-scope", context_repl);
}
