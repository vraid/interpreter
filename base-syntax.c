#include "base-syntax.h"

#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "base-util.h"
#include "list-util.h"
#include "call.h"
#include "memory-handling.h"
#include "environments.h"
#include "eval.h"

object bind_value_proc;

object* bind_value(object* args, object* cont) {
	object* value;
	object* name;
	object* environment;
	delist_3(args, &value, &name, &environment);
	
	if (is_internal_error(value)) {
		return call_cont(cont, value);
	}
	else {
		object call;
		init_call(&call, &extend_environment_proc, args, cont);
		return perform_call(&call);
	}
}

object* define(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* name;
	object* body;
	delist_2(syntax, &name, &body);
	
	object ls[2];
	init_list_2(ls, name, environment);
	object bind_call;
	init_call(&bind_call, &bind_value_proc, ls, cont);
	object bind_cont;
	init_cont(&bind_cont, &bind_call);
	
	object ls2[2];
	init_list_2(ls2, body, environment);
	object call;
	init_call(&call, eval_proc(), ls2, &bind_cont);
	return perform_call(&call);
}

object* quote(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object call;
	init_call(&call, quote_object_proc(), syntax, cont);
	
	return perform_call(&call);
}

object* lambda(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* parameters;
	object* body;
	delist_2(syntax, &parameters, &body);
	
	object function;
	init_function(&function, environment, parameters, body);
	
	return call_cont(cont, &function);
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
		object par[1];
		init_list_1(par, list_first(parameters));
		object syntax[3];
		init_list_3(syntax, lambda_symbol(), par, body);
		
		object ls[2];
		init_list_2(ls, list_rest(parameters), syntax);
		object call;
		init_call(&call, &curry_one_proc, ls, cont);
		
		return perform_call(&call);
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
		object ls[1];
		init_list_1(ls, function_body(function));
		object curry_call;
		init_call(&curry_call, &curry_one_proc, ls, cont);
		
		object curry_cont;
		init_cont(&curry_cont, &curry_call);
		
		object ls2[1];
		init_list_1(ls2, parameters);
		object reverse_call;
		init_call(&reverse_call, reverse_list_proc(), ls2, &curry_cont);
		
		return perform_call(&reverse_call);
	}
}

object* curry(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* function;
	delist_1(syntax, &function);
	
	object evalls[1];
	init_list_1(evalls, environment);
	object eval_curry;
	init_call(&eval_curry, eval_proc(), evalls, cont);
	object eval_cont;
	init_cont(&eval_cont, &eval_curry);

	object curry_call;
	init_call(&curry_call, &start_curry_proc, empty_list(), &eval_cont);
	object curry_cont;
	init_cont(&curry_cont, &curry_call);
	
	object ls[2];
	init_list_2(ls, function, environment);
	object eval_call;
	init_call(&eval_call, eval_proc(), ls, &curry_cont);
	
	return perform_call(&eval_call);
}

object* apply(object* args, object* cont) {
	return no_object();
}

object eval_if_proc;

object* eval_if(object* args, object* cont) {
	object* condition;
	object* then;
	object* els;
	object* environment;
	delist_4(args, &condition, &then, &els, &environment);
	
	object ls[2];
	object call;
	
	if (is_false(condition)) {
		init_list_2(ls, els, environment);
	}
	else {
		init_list_2(ls, then, environment);
	}
	
	init_call(&call, eval_proc(), ls, cont);
	return perform_call(&call);
}

object* if_func(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* condition;
	object* then;
	object* els;
	delist_3(syntax, &condition, &then, &els);
	
	object ls[3];
	init_list_3(ls, then, els, environment);
	object next_call;
	init_call(&next_call, &eval_if_proc, ls, cont);
	object next_cont;
	init_cont(&next_cont, &next_call);
	
	object ls2[2];
	init_list_2(ls2, condition, environment);
	object call;
	init_call(&call, eval_proc(), ls2, &next_cont);
	return perform_call(&call);
}

object* list(object* args, object* cont) {
	object* elements;
	object* environment;
	delist_2(args, &elements, &environment);
	
	object quote_call;
	init_call(&quote_call, quote_object_proc(), empty_list(), cont);
	
	object quote_cont;
	init_cont(&quote_cont, &quote_call);
	
	object eval_call;
	init_call(&eval_call, eval_list_elements_proc(), args, &quote_cont);
	
	return perform_call(&eval_call);
}

object* map(object* args, object* cont) {
	return no_object();
}

object* fold(object* args, object* cont) {
	return no_object();
}

object* filter(object* args, object* cont) {
	return no_object();
}

void add_syntax(char* name, static_syntax syntax, primitive_proc* proc) {
	init_syntax(&syntax_procedure[syntax], proc);
	add_static_binding(&syntax_procedure[syntax], name);
}

void init_base_syntax_procedures(void) {	
	add_syntax("define", syntax_define, &define);
	add_syntax("quote", syntax_quote, &quote);
	add_syntax("lambda", syntax_lambda, &lambda);
	add_syntax("curry", syntax_curry, &curry);
	add_syntax("apply", syntax_apply, &apply);
	add_syntax("if", syntax_if, &if_func);
	add_syntax("list", syntax_list, &list);
	add_syntax("map", syntax_map, &map);
	add_syntax("fold", syntax_fold, &fold);
	add_syntax("filter", syntax_filter, &filter);
	
	init_primitive_procedure(&bind_value_proc, &bind_value);
	init_primitive_procedure(&eval_if_proc, &eval_if);
	
	init_primitive_procedure(&start_curry_proc, &start_curry);
	init_primitive_procedure(&curry_one_proc, &curry_one);
}
