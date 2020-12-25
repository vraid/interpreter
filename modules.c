#include "modules.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "memory-handling.h"
#include "delist.h"
#include "base-util.h"
#include "list-util.h"
#include "call.h"
#include "environments.h"
#include "syntax-base.h"
#include "standard-library.h"
#include "eval.h"

object module_eval_interims_proc;

object* module_eval_interims(object* args, object* cont) {
	object* interims;
	object* environment;
	object* trace;
	delist_3(args, &interims, &environment, &trace);
	
	if (is_empty_list(interims)) {
		object* module = alloc_module(list_first(environment));
		return call_cont(cont, module);
	}
	else {
		object* next_args = alloc_list_3(list_rest(interims), environment, trace);
		object* next_call = alloc_call(&module_eval_interims_proc, next_args, cont);
		
		object* interim = list_first(interims);
		object* interim_cont = module_interim_cont(interim);
		interim_cont->data.continuation.call = next_call;
		alloc_mutation_reference(interim_cont, next_call);		
		
		return perform_call(module_interim_call(interim));
	}
}

object module_next_proc;

object* module_next(object* args, object* cont) {
	object* interims;
	object* environment;
	object* trace;
	delist_3(args, &interims, &environment, &trace);
	
	// interims changed below
	object* next_args = alloc_list_3(interims, environment, trace);
	object* next_call = alloc_call(&module_eval_interims_proc, next_args, cont);
	
	object* bindings = empty_list();
	while (!is_empty_list(interims)) {
		object* interim = list_first(interims);
		object* ls = module_interim_bindings(interim);
		while (!is_empty_list(ls)) {
			bindings = alloc_list_cell(list_first(ls), bindings);
			ls = list_rest(ls);
		}
		interims = list_rest(interims);
	}
	
	environment->data.list.first = bindings;
	alloc_mutation_reference(environment, bindings);
	
	return perform_call(next_call);
}

object module_eval_expressions_proc;

object* module_eval_expressions(object* args, object* cont) {
	object* interims;
	object* syntax;
	object* environment;
	object* trace;
	delist_4(args, &interims, &syntax, &environment, &trace);
	
	if (is_empty_list(syntax)) {
		return call_cont(cont, interims);
	}
	else {
		object* expr = list_first(syntax);
		
		object* next_args = alloc_list_3(list_rest(syntax), environment, trace);
		object* next_call = alloc_call(&module_eval_expressions_proc, next_args, cont);
		
		object* link_call = alloc_call(&link_list_proc, alloc_list_1(interims), alloc_cont(next_call));
		
		object* eval_args = alloc_list_4(expr, environment, module_context(), trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(link_call));
		
		return perform_call(eval_call);
	}
}

object* module(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* module_env = alloc_list_cell(empty_list(), environment);
	
	object* next_args = alloc_list_2(module_env, trace);
	object* next_call = alloc_call(&module_next_proc, next_args, cont);
	
	object* reverse_call = alloc_call(&reverse_list_proc, empty_list(), alloc_cont(next_call));
	
	object* eval_args = alloc_list_4(empty_list(), syntax, module_env, trace);
	object* eval_call = alloc_call(&module_eval_expressions_proc, eval_args, alloc_cont(reverse_call));
	
	return perform_call(eval_call);
}

object using_next_proc;

object* using_next(object* args, object* cont) {
	object* modules;
	object* body;
	object* environment;
	object* trace;
	delist_4(args, &modules, &body, &environment, &trace);
	
	object* new_env = environment;
	while (!is_empty_list(modules)) {
		object* module = desyntax(list_first(modules));
		if (!is_module(module)) {
			return throw_trace_error_string(cont, trace, "using of non-module");
		}
		new_env = alloc_list_cell(module_bindings(module), new_env);
		modules = list_rest(modules);
	}
	
	object* eval_args = alloc_list_4(body, new_env, scope_context(), trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object* using(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* modules;
	object* body;
	delist_2(syntax, &modules, &body);
	
	object* next_args = alloc_list_3(body, environment, trace);
	object* next_call = alloc_call(&using_next_proc, next_args, cont);
	
	object* eval_args = alloc_list_4(desyntax(modules), environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, alloc_cont(next_call));
	
	return perform_call(eval_call);
}

object* define(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* signature;
	object* body;
	delist_2(syntax, &signature, &body);
	
	signature = desyntax(signature);
	
	// handles cases like (define ((f a) b) ..)
	while (is_list(signature)) {
		body = alloc_list_3(syntax_procedure_obj(syntax_lambda), list_rest(signature), body);
		signature = desyntax(list_first(signature));
	}
	
	object* binding = alloc_placeholder_binding(signature);
	
	object* return_cont = alloc_discarding_cont(no_object());
	object* update_call = alloc_call(&update_binding_proc, alloc_list_1(binding), return_cont);
	
	object* eval_args = alloc_list_4(body, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(update_call));
	
	object* bindings = alloc_list_1(binding);
	object* res = alloc_module_interim(bindings, eval_call, return_cont);
	
	return call_cont(cont, res);
}

void init_module_procedures(void) {
	add_syntax("module", syntax_module, context_value, &module);
	add_syntax("using", syntax_using, context_value, &using);
	add_syntax("define", syntax_define, context_module, &define);
	
	init_primitive(&module_eval_interims, &module_eval_interims_proc);
	init_primitive(&module_next, &module_next_proc);
	init_primitive(&module_eval_expressions, &module_eval_expressions_proc);
	init_primitive(&using_next, &using_next_proc);
}
