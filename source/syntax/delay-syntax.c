#include "delay-syntax.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "list-util.h"
#include "memory-handling.h"
#include "syntax-base.h"
#include "environments.h"
#include "call.h"
#include "eval.h"

object* delay(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* value;
	delist_1(syntax, &value);
	
	object* delay_args = alloc_list_3(value, environment, trace);	
	object* obj = alloc_delay(delay_args);
	
	return call_cont(cont, obj);
}

object update_delay_proc;

object* update_delay(object* args, object* cont) {
	object* value;
	object* delay;
	delist_2(args, &value, &delay);
	
	delay->data.delay.value = value;
	delay->data.delay.evaluated = 1;
	
	alloc_mutation_reference(delay, value);
	
	return call_cont(cont, value);
}

object* eval_force(object* args, object* cont) {
	object* obj;
	delist_1(args, &obj);
	
	if (!is_delay(obj)) {
		return call_cont(cont, obj);
	}
	else if (delay_evaluated(obj)) {
		return call_cont(cont, delay_value(obj));
	}
	else {
		object* update_args = alloc_list_1(obj);
		object* update_call = alloc_call(&update_delay_proc, update_args, cont);
		
		object* eval_call = alloc_call(&eval_proc, delay_value(obj), alloc_cont(update_call));
		
		return perform_call(eval_call);
	}
}

object* force(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* value;
	delist_1(syntax, &value);
	
	object* force_call = alloc_call(&eval_force_proc, empty_list(), cont);
	
	object* eval_args = alloc_list_4(value, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(force_call));
	
	return perform_call(eval_call);
}

void init_delay_syntax_procedures(void) {
	add_syntax("delay", syntax_delay, context_value, &delay);
	add_syntax("force", syntax_force, context_value, &force);
	
	init_primitive(&update_delay, &update_delay_proc);
	init_primitive(&eval_force, &eval_force_proc);
}
