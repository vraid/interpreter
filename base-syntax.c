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
	
	object call;
	init_call(&call, &extend_environment_proc, args, cont);
	return perform_call(&call);
}

object update_binding_proc;

object* update_binding(object* args, object* cont) {
	object* value;
	object* name;
	object* environment;
	delist_3(args, &value, &name, &environment);
	
	object* binding = find_in_environment(environment, name, 1);
	
	if (is_no_binding(binding) || !is_placeholder_value(binding_value(binding))) {
		return throw_error(cont, "updating non-placeholder binding");
	}
	binding->data.binding.value = value;
	add_mutation(binding, value);
	
	// the environment is not the latest one, so it is discarded
	return call_discarding_cont(cont);
}

object bind_continued_proc;

object* bind_continued(object* args, object* cont) {
	object* environment;
	object* body;
	object* name;
	delist_3(args, &environment, &body, &name);
	
	object return_args[1];
	init_list_1(return_args, environment);
	object return_call;
	init_call(&return_call, identity_proc(), return_args, cont);
	object return_cont;
	init_discarding_cont(&return_cont, &return_call);
	
	object update_args[2];
	init_list_2(update_args, name, environment);
	object update_call;
	init_call(&update_call, &update_binding_proc, update_args, &return_cont);
	object update_cont;
	init_cont(&update_cont, &update_call);
	
	object eval_args[2];
	init_list_2(eval_args, body, environment);
	object eval_call;
	init_call(&eval_call, eval_proc(), eval_args, &update_cont);
	
	return perform_call(&eval_call);
}

object bind_placeholder_proc;

object* bind_placeholder(object* args, object* cont) {
	object* environment;
	object* name;
	delist_2(args, &environment, &name);
	
	object bind_args[3];
	init_list_3(bind_args, placeholder_value(), name, environment);
	object bind_call;
	init_call(&bind_call, &extend_environment_proc, bind_args, cont);
	
	return perform_call(&bind_call);
}

object* define(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* name;
	object* body;
	delist_2(syntax, &name, &body);
	
	object continued_args[2];
	init_list_2(continued_args, body, name);
	object bind_continued_call;
	init_call(&bind_continued_call, &bind_continued_proc, continued_args, cont);
	object continued_cont;
	init_cont(&continued_cont, &bind_continued_call);
	
	object bind_args[2];
	init_list_2(bind_args, environment, name);
	object bind_call;
	init_call(&bind_call, &bind_placeholder_proc, bind_args, &continued_cont);
	object bind_cont;
	init_cont(&bind_cont, &bind_call);
	
	return perform_call(&bind_call);
}

object* quote(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object call;
	init_call(&call, quote_object_proc(), syntax, cont);
	
	return perform_call(&call);
}

object let_bind_proc;

object* let_bind(object* args, object* cont) {
	object* environment;
	object* bindings;
	delist_2(args, &environment, &bindings);
	
	if (is_empty_list(bindings)) {
		return call_cont(cont, environment);
	}
	else {
		object* first = list_first(bindings);
		object* name;
		object* value;
		delist_2(first, &name, &value);
		
		if (!is_symbol(name)) {
			return throw_error(cont, "not a valid identifier");
		}
		
		object let_args[1];
		init_list_1(let_args, list_rest(bindings));
		object let_call;
		init_call(&let_call, &let_bind_proc, let_args, cont);
		object let_cont;
		init_cont(&let_cont, &let_call);
		
		object bind_args[2];
		init_list_2(bind_args, name, environment);
		object bind_call;
		init_call(&bind_call, &extend_environment_proc, bind_args, &let_cont);
		object bind_cont;
		init_cont(&bind_cont, &bind_call);
		
		object eval_args[2];
		init_list_2(eval_args, value, environment);
		object eval_call;
		init_call(&eval_call, eval_proc(), eval_args, &bind_cont);
		
		return perform_call(&eval_call);
	}
}

object* let(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* bindings;
	object* body;
	delist_2(syntax, &bindings, &body);
	
	object eval_args[1];
	init_list_1(eval_args, body);
	object eval_call;
	init_call(&eval_call, eval_with_environment_proc(), eval_args, cont);
	object eval_cont;
	init_cont(&eval_cont, &eval_call);
	
	object let_args[2];
	init_list_2(let_args, environment, bindings);
	object let_call;
	init_call(&let_call, &let_bind_proc, let_args, &eval_cont);
	
	return perform_call(&let_call);
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
	
	object eval_ls[1];
	init_list_1(eval_ls, environment);
	object eval_curry;
	init_call(&eval_curry, eval_proc(), eval_ls, cont);
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

object start_apply_proc;

object* start_apply(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* function = list_first(syntax);
	object* values = list_rest(syntax);
	
	object* parameters = function_parameters(function);
	
	//pick off the parameters left for the body lambda
	object* vs = values;
	object* ps = parameters;
	while (!is_empty_list(vs)) {
		vs = list_rest(vs);
		ps = list_rest(ps);
	}
	
	object body_ls[3];
	init_list_3(body_ls, lambda_symbol(), ps, function_body(function));
	object eval_ls[1];
	init_list_1(eval_ls, body_ls);
	object eval_call;
	init_call(&eval_call, eval_with_environment_proc(), eval_ls, cont);
	object eval_cont;
	init_cont(&eval_cont, &eval_call);
	
	object bind_ls[3];
	init_list_3(bind_ls, values, parameters, environment);
	object bind_call;
	init_call(&bind_call, bind_values_proc(), bind_ls, &eval_cont);
	
	return perform_call(&bind_call);
}

object* apply(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object apply_ls[1];
	init_list_1(apply_ls, environment);
	object apply_call;
	init_call(&apply_call, &start_apply_proc, apply_ls, cont);
	object apply_cont;
	init_cont(&apply_cont, &apply_call);
	
	object eval_call;
	init_call(&eval_call, eval_list_elements_proc(), args, &apply_cont);
	
	return perform_call(&eval_call);
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

object eval_and_proc;

object* eval_and(object* args, object* cont) {
	object* value;
	object* elements;
	object* environment;
	delist_3(args, &value, &elements, &environment);
	
	if (is_false(value)) {
		return call_cont(cont, false());
	}
	else if (is_empty_list(elements)) {
		return call_cont(cont, value);
	}
	else {
		object and_args[2];
		init_list_2(and_args, list_rest(elements), environment);
		object and_call;
		init_call(&and_call, &eval_and_proc, and_args, cont);
		object and_cont;
		init_cont(&and_cont, &and_call);
		
		object eval_args[2];
		init_list_2(eval_args, list_first(elements), environment);
		object eval_call;
		init_call(&eval_call, eval_proc(), eval_args, &and_cont);
		
		return perform_call(&eval_call);
	}
}

object* and(object* args, object* cont) {
	object* elements;
	object* environment;
	delist_2(args, &elements, &environment);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, true());
	}
	else {
		object and_args[2];
		init_list_2(and_args, list_rest(elements), environment);
		object and_call;
		init_call(&and_call, &eval_and_proc, and_args, cont);
		object and_cont;
		init_cont(&and_cont, &and_call);
		
		object eval_args[2];
		init_list_2(eval_args, list_first(elements), environment);
		object eval_call;
		init_call(&eval_call, eval_proc(), eval_args, &and_cont);
		
		return perform_call(&eval_call);
	}
}

object eval_or_proc;

object* eval_or(object* args, object* cont) {
	object* value;
	object* elements;
	object* environment;
	delist_3(args, &value, &elements, &environment);
	
	if (!is_false(value)) {
		return call_cont(cont, value);
	}
	else if (is_empty_list(elements)) {
		return call_cont(cont, false());
	}
	else {
		object or_args[2];
		init_list_2(or_args, list_rest(elements), environment);
		object or_call;
		init_call(&or_call, &eval_or_proc, or_args, cont);
		object or_cont;
		init_cont(&or_cont, &or_call);
		
		object eval_args[2];
		init_list_2(eval_args, list_first(elements), environment);
		object eval_call;
		init_call(&eval_call, eval_proc(), eval_args, &or_cont);
		
		return perform_call(&eval_call);
	}
}

object* or(object* args, object* cont) {
	object* elements;
	object* environment;
	delist_2(args, &elements, &environment);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, false());
	}
	else {
		object or_args[2];
		init_list_2(or_args, list_rest(elements), environment);
		object or_call;
		init_call(&or_call, &eval_or_proc, or_args, cont);
		object or_cont;
		init_cont(&or_cont, &or_call);
		
		object eval_args[2];
		init_list_2(eval_args, list_first(elements), environment);
		object eval_call;
		init_call(&eval_call, eval_proc(), eval_args, &or_cont);
		
		return perform_call(&eval_call);
	}
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

object map_single_proc;

object* map_single(object* args, object* cont) {
	object* last;
	object* unmapped;
	object* function;
	delist_3(args, &last, &unmapped, &function);
	
	if (is_empty_list(unmapped)) {
		return call_discarding_cont(cont);
	}
	else {
		object map_args[2];
		init_list_2(map_args, list_rest(unmapped), function);
		object map_call;
		init_call(&map_call, &map_single_proc, map_args, cont);
		object map_cont;
		init_cont(&map_cont, &map_call);
		
		object add_args[1];
		init_list_1(add_args, last);
		object add_call;
		init_call(&add_call, add_to_list_proc(), add_args, &map_cont);
		object add_cont;
		init_cont(&add_cont, &add_call);
		
		object function_args[1];
		init_list_1(function_args, list_first(unmapped));
		object eval_args[2];
		init_list_2(eval_args, function_args, function);
		object eval_call;
		init_call(&eval_call, eval_function_call_proc(), eval_args, &add_cont);
		
		return perform_call(&eval_call);
	}
}

object map_start_proc;

object* map_start(object* args, object* cont) {
	object* syntax;
	delist_1(args, &syntax);
	
	object* function;
	object* elements;
	delist_2(syntax, &function, &elements);
	
	elements = unquote(elements);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, empty_list());
	}
	else {
		object map_args[2];
		init_list_2(map_args, list_rest(elements), function);
		object list_args[2];
		init_list_2(list_args, &map_single_proc, map_args);
		object list_call;
		init_call(&list_call, make_list_proc(), list_args, cont);
		object list_cont;
		init_cont(&list_cont, &list_call);
		
		object function_args[1];
		init_list_1(function_args, list_first(elements));
		object eval_args[2];
		init_list_2(eval_args, function_args, function);
		object eval_call;
		init_call(&eval_call, eval_function_call_proc(), eval_args, &list_cont);
		
		return perform_call(&eval_call);
	}
}

object* map(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object quote_call;
	init_call(&quote_call, quote_object_proc(), empty_list(), cont);
	object quote_cont;
	init_cont(&quote_cont, &quote_call);
	
	object start_call;
	init_call(&start_call, &map_start_proc, empty_list(), &quote_cont);
	object start_cont;
	init_cont(&start_cont, &start_call);
	
	object eval_args[2];
	init_list_2(eval_args, syntax, environment);
	object eval_call;
	init_call(&eval_call, eval_list_elements_proc(), eval_args, &start_cont);
	
	return perform_call(&eval_call);
}

object fold_single_proc;

object* fold_single(object* args, object* cont) {
	object* value;
	object* elements;
	object* function;
	delist_3(args, &value, &elements, &function);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, value);
	}
	else {
		object fold_args[2];
		init_list_2(fold_args, list_rest(elements), function);
		object fold_call;
		init_call(&fold_call, &fold_single_proc, fold_args, cont);
		object fold_cont;
		init_cont(&fold_cont, &fold_call);
		
		object function_args[2];
		init_list_2(function_args, value, list_first(elements));
		object eval_args[2];
		init_list_2(eval_args, function_args, function);
		object eval_call;
		init_call(&eval_call, eval_function_call_proc(), eval_args, &fold_cont);
		
		return perform_call(&eval_call);
	}
}

object fold_start_proc;

object* fold_start(object* args, object* cont) {
	object* syntax;
	delist_1(args, &syntax);
	
	object* function;
	object* initial;
	object* elements;
	delist_3(syntax, &function, &initial, &elements);
	
	elements = unquote(elements);
	
	object fold_args[3];
	init_list_3(fold_args, initial, elements, function);
	object fold_call;
	init_call(&fold_call, &fold_single_proc, fold_args, cont);
	
	return perform_call(&fold_call);
}

object* fold(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object fold_call;
	init_call(&fold_call, &fold_start_proc, empty_list(), cont);
	object fold_cont;
	init_cont(&fold_cont, &fold_call);
	
	object eval_args[2];
	init_list_2(eval_args, syntax, environment);
	object eval_call;
	init_call(&eval_call, eval_list_elements_proc(), eval_args, &fold_cont);
	
	return perform_call(&eval_call);
}

object add_or_discard_filtered_proc;

object* add_or_discard_filtered(object* args, object* cont) {
	object* result;
	object* value;
	object* last;
	delist_3(args, &result, &value, &last);
	
	if (is_false(result)) {
		return call_cont(cont, last);
	}
	else {
		object add_args[2];
		init_list_2(add_args, value, last);
		object add_call;
		init_call(&add_call, add_to_list_proc(), add_args, cont);
		
		return perform_call(&add_call);
	}
}

object filter_single_proc;

object* filter_single(object* args, object* cont) {
	object* last;
	object* unfiltered;
	object* function;
	delist_3(args, &last, &unfiltered, &function);
	
	if (is_empty_list(unfiltered)) {
		return call_discarding_cont(cont);
	}
	else {
		object filter_args[2];
		init_list_2(filter_args, list_rest(unfiltered), function);
		object filter_call;
		init_call(&filter_call, &filter_single_proc, filter_args, cont);
		object filter_cont;
		init_cont(&filter_cont, &filter_call);
		
		object* value = list_first(unfiltered);
		object add_args[2];
		init_list_2(add_args, value, last);
		object add_call;
		init_call(&add_call, &add_or_discard_filtered_proc, add_args, &filter_cont);
		object add_cont;
		init_cont(&add_cont, &add_call);
		
		object function_args[1];
		init_list_1(function_args, value);
		object eval_args[2];
		init_list_2(eval_args, function_args, function);
		object eval_call;
		init_call(&eval_call, eval_function_call_proc(), eval_args, &add_cont);
		
		return perform_call(&eval_call);
	}
}

object filter_first_proc;

object* filter_first(object* args, object* cont) {
	object* result;
	object* value;
	object* unfiltered;
	object* function;
	delist_4(args, &result, &value, &unfiltered, &function);
	
	// continue without starting a list
	if (is_false(result)) {
		if (is_empty_list(unfiltered)) {
			return call_cont(cont, empty_list());
		}
		else {
			object* value = list_first(unfiltered);
			object filter_args[3];
			init_list_3(filter_args, value, list_rest(unfiltered), function);
			object filter_call;
			init_call(&filter_call, &filter_first_proc, filter_args, cont);
			object filter_cont;
			init_cont(&filter_cont, &filter_call);
			
			object function_args[1];
			init_list_1(function_args, value);
			object eval_args[2];
			init_list_2(eval_args, function_args, function);
			object eval_call;
			init_call(&eval_call, eval_function_call_proc(), eval_args, &filter_cont);
			
			return perform_call(&eval_call);
		}
	}
	// start making a list
	else {
		object filter_args[2];
		init_list_2(filter_args, unfiltered, function);
		object list_args[3];
		init_list_3(list_args, value, &filter_single_proc, filter_args);
		object list_call;
		init_call(&list_call, make_list_proc(), list_args, cont);
		
		return perform_call(&list_call);
	}
}

object filter_start_proc;

object* filter_start(object* args, object* cont) {
	object* syntax;
	delist_1(args, &syntax);
	
	object* function;
	object* elements;
	delist_2(syntax, &function, &elements);
	
	elements = unquote(elements);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, empty_list());
	}
	else {
		object* value = list_first(elements);
		object filter_args[3];
		init_list_3(filter_args, value, list_rest(elements), function);
		object filter_call;
		init_call(&filter_call, &filter_first_proc, filter_args, cont);
		object filter_cont;
		init_cont(&filter_cont, &filter_call);
		
		object function_args[1];
		init_list_1(function_args, value);
		object eval_args[2];
		init_list_2(eval_args, function_args, function);
		object eval_call;
		init_call(&eval_call, eval_function_call_proc(), eval_args, &filter_cont);
		
		return perform_call(&eval_call);
	}
}

object* filter(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object quote_call;
	init_call(&quote_call, quote_object_proc(), empty_list(), cont);
	object quote_cont;
	init_cont(&quote_cont, &quote_call);
	
	object start_call;
	init_call(&start_call, &filter_start_proc, empty_list(), &quote_cont);
	object start_cont;
	init_cont(&start_cont, &start_call);
	
	object eval_args[2];
	init_list_2(eval_args, syntax, environment);
	object eval_call;
	init_call(&eval_call, eval_list_elements_proc(), eval_args, &start_cont);
	
	return perform_call(&eval_call);
}

void add_syntax(char* name, static_syntax syntax, primitive_proc* proc) {
	init_syntax(&syntax_procedure[syntax], proc);
	add_static_binding(&syntax_procedure[syntax], name);
}

void init_base_syntax_procedures(void) {	
	add_syntax("define", syntax_define, &define);
	add_syntax("quote", syntax_quote, &quote);
	add_syntax("let", syntax_let, &let);
	add_syntax("lambda", syntax_lambda, &lambda);
	add_syntax("curry", syntax_curry, &curry);
	add_syntax("apply", syntax_apply, &apply);
	add_syntax("if", syntax_if, &if_func);
	add_syntax("and", syntax_and, &and);
	add_syntax("or", syntax_or, &or);
	add_syntax("list", syntax_list, &list);
	add_syntax("map", syntax_map, &map);
	add_syntax("fold", syntax_fold, &fold);
	add_syntax("filter", syntax_filter, &filter);
	
	init_primitive_procedure(&let_bind_proc, &let_bind);
	
	init_primitive_procedure(&bind_value_proc, &bind_value);
	init_primitive_procedure(&eval_if_proc, &eval_if);
	
	init_primitive_procedure(&eval_and_proc, &eval_and);
	init_primitive_procedure(&eval_or_proc, &eval_or);
	
	init_primitive_procedure(&bind_placeholder_proc, &bind_placeholder);
	init_primitive_procedure(&bind_continued_proc, &bind_continued);
	init_primitive_procedure(&update_binding_proc, &update_binding);
	
	init_primitive_procedure(&start_curry_proc, &start_curry);
	init_primitive_procedure(&curry_one_proc, &curry_one);
	
	init_primitive_procedure(&start_apply_proc, &start_apply);
	
	init_primitive_procedure(&map_single_proc, &map_single);
	init_primitive_procedure(&map_start_proc, &map_start);
	
	init_primitive_procedure(&filter_start_proc, &filter_start);
	init_primitive_procedure(&filter_first_proc, &filter_first);
	init_primitive_procedure(&filter_single_proc, &filter_single);
	init_primitive_procedure(&add_or_discard_filtered_proc, &add_or_discard_filtered);
	
	init_primitive_procedure(&fold_start_proc, &fold_start);
	init_primitive_procedure(&fold_single_proc, &fold_single);
}
