#include "logic-syntax.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "list-util.h"
#include "syntax-base.h"
#include "call.h"
#include "eval.h"

object eval_if_proc;

object* eval_if(object* args, object* cont) {
	object* condition;
	object* then;
	object* els;
	object* environment;
	object* trace;
	delist_5(args, &condition, &then, &els, &environment, &trace);
	
	object* ls = alloc_list_4(is_false(condition) ? els : then, environment, default_context(), trace);
	object* call = alloc_call(&eval_proc, ls, cont);
	
	return perform_call(call);
}

object* if_func(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* condition;
	object* then;
	object* els;
	delist_3(syntax, &condition, &then, &els);
	
	object* next_args = alloc_list_4(then, els, environment, trace);
	object* next_call = alloc_call(&eval_if_proc, next_args, cont);
	
	object* call_args = alloc_list_4(condition, environment, default_context(), trace);
	object* call = alloc_call(&eval_proc, call_args, alloc_cont(next_call));
	return perform_call(call);
}

object eval_and_proc;

object* eval_and(object* args, object* cont) {
	object* value;
	object* elements;
	object* environment;
	object* trace;
	delist_4(args, &value, &elements, &environment, &trace);
	
	if (is_false(value)) {
		return call_cont(cont, false());
	}
	else if (is_empty_list(elements)) {
		return call_cont(cont, value);
	}
	else {
		object* and_args = alloc_list_3(list_rest(elements), environment, trace);
		object* and_call = alloc_call(&eval_and_proc, and_args, cont);
		
		object* eval_args = alloc_list_4(list_first(elements), environment, default_context(), trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(and_call));
		
		return perform_call(eval_call);
	}
}

object* and(object* args, object* cont) {
	object* elements;
	object* environment;
	object* trace;
	delist_3(args, &elements, &environment, &trace);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, true());
	}
	else {
		object* and_args = alloc_list_3(list_rest(elements), environment, trace);
		object* and_call = alloc_call(&eval_and_proc, and_args, cont);
		
		object* eval_args = alloc_list_4(list_first(elements), environment, default_context(), trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(and_call));
		
		return perform_call(eval_call);
	}
}

object eval_or_proc;

object* eval_or(object* args, object* cont) {
	object* value;
	object* elements;
	object* environment;
	object* trace;
	delist_4(args, &value, &elements, &environment, &trace);
	
	if (!is_false(value)) {
		return call_cont(cont, value);
	}
	else if (is_empty_list(elements)) {
		return call_cont(cont, false());
	}
	else {
		object* or_args = alloc_list_3(list_rest(elements), environment, trace);
		object* or_call = alloc_call(&eval_or_proc, or_args, cont);
		
		object* eval_args = alloc_list_4(list_first(elements), environment, default_context(), trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(or_call));
		
		return perform_call(eval_call);
	}
}

object* or(object* args, object* cont) {
	object* elements;
	object* environment;
	object* trace;
	delist_3(args, &elements, &environment, &trace);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, false());
	}
	else {
		object* or_args = alloc_list_3(list_rest(elements), environment, trace);
		object* or_call = alloc_call(&eval_or_proc, or_args, cont);
		
		object* eval_args = alloc_list_4(list_first(elements), environment, default_context(), trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(or_call));
		
		return perform_call(eval_call);
	}
}

void init_logic_syntax_procedures(void) {
	add_syntax("if", syntax_if, context_value, &if_func);
	add_syntax("and", syntax_and, context_value, &and);
	add_syntax("or", syntax_or, context_value, &or);

	init_primitive(&eval_if, &eval_if_proc);
	
	init_primitive(&eval_and, &eval_and_proc);
	init_primitive(&eval_or, &eval_or_proc);
}
