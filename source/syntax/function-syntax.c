#include "function-syntax.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "list-util.h"
#include "syntax-base.h"
#include "environments.h"
#include "call.h"
#include "eval.h"

object* lambda(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* parameters;
	object* body;
	delist_2(syntax, &parameters, &body);
	
	parameters = desyntax(parameters);
	
	object* dup = find_duplicate(parameters);
	if (!is_false(dup)) {
		object* str = alloc_string("duplicate parameter");
		object* ls = alloc_list_2(str, syntax);
		return throw_trace_error(cont, trace, ls);
	}
	else {
		object* function = alloc_function(environment, parameters, body);
		
		return call_cont(cont, function);
	}
}

object curry_one_proc;

object* curry_one(object* args, object* cont) {
	object* parameters;
	object* body;
	delist_2(args, &parameters, &body);
	
	if (is_empty_list(parameters)) {
		return call_cont(cont, body);
	}
	else {
		object* par = alloc_list_1(list_first(parameters));
		object* syntax = alloc_list_3(syntax_procedure_obj(syntax_lambda), par, body);
		
		object* ls = alloc_list_2(list_rest(parameters), syntax);
		object* call = alloc_call(&curry_one_proc, ls, cont);
		
		return perform_call(call);
	}
}

object start_curry_proc;

object* start_curry(object* args, object* cont) {
	object* function;
	delist_1(args, &function);
	
	object* parameters = function_parameters(function);
	
	if (is_empty_list(parameters)) {
		return call_cont(cont, function);
	}
	else {
		object* curry_args = alloc_list_1(function_body(function));
		object* curry_call = alloc_call(&curry_one_proc, curry_args, cont);
		
		object* reverse_args = alloc_list_1(parameters);
		object* reverse_call = alloc_call(&reverse_list_proc, reverse_args, alloc_cont(curry_call));
		
		return perform_call(reverse_call);
	}
}

object* curry(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* function;
	delist_desyntax_1(syntax, &function);
	
	object* eval_args = alloc_list_3(environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, cont);

	object* curry_call = alloc_call(&start_curry_proc, empty_list(), alloc_cont(eval_call));
	
	object* eval_function_args = alloc_list_4(function, environment, default_context(), trace);
	object* eval_function_call = alloc_call(&eval_proc, eval_function_args, alloc_cont(curry_call));
	
	return perform_call(eval_function_call);
}

object start_apply_proc;

object* start_apply(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* function = list_first(syntax);
	object* values = list_rest(syntax);
	
	object* parameters = function_parameters(function);
	
	object* vs = values;
	object* vals = empty_list();
	
	object* ps = parameters;	
	object* pars = empty_list();
	while (!is_empty_list(vs)) {
		// alloc values / names to bind
		vals = alloc_list_cell(list_first(vs), vals);
		pars = alloc_list_cell(list_first(ps), pars);
		// pick off the parameters left for the body lambda
		vs = list_rest(vs);
		ps = list_rest(ps);
	}
	
	object* body = alloc_list_3(syntax_procedure_obj(syntax_lambda), ps, function_body(function));
	object* eval_args = alloc_list_3(body, default_context(), trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	object* bind_args = alloc_list_3(vals, pars, environment);
	object* bind_call = alloc_call(&bind_values_proc, bind_args, alloc_cont(eval_call));
	
	return perform_call(bind_call);
}

object* apply(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* apply_args = alloc_list_2(environment, trace);
	object* apply_call = alloc_call(&start_apply_proc, apply_args, cont);
	
	object* eval_args = alloc_list_4(syntax, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, alloc_cont(apply_call));
	
	return perform_call(eval_call);
}

void init_function_syntax_procedures(void) {
	add_syntax("lambda", syntax_lambda, context_value, &lambda);
	add_syntax("curry", syntax_curry, context_value, &curry);
	add_syntax("apply", syntax_apply, context_value, &apply);
	
	init_primitive(&start_curry, &start_curry_proc);
	init_primitive(&curry_one, &curry_one_proc);
	
	init_primitive(&start_apply, &start_apply_proc);
}
