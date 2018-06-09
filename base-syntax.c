#include "base-syntax.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "base-util.h"
#include "list-util.h"
#include "call.h"
#include "memory-handling.h"
#include "environments.h"
#include "standard-library.h"
#include "eval.h"
#include "structs.h"

object bind_value_proc;

object* bind_value(object* args, object* cont) {
	object* value;
	object* name;
	object* environment;
	delist_3(args, &value, &name, &environment);
	
	object* call = alloc_call(&extend_environment_proc, args, cont);
	return perform_call(call);
}

object update_binding_proc;

object* update_binding(object* args, object* cont) {
	object* value;
	object* name;
	object* environment;
	delist_3(args, &value, &name, &environment);
	
	object* binding = find_in_environment(environment, name, 1);
	
	if (is_no_binding(binding) || !is_placeholder_value(binding_value(binding))) {
		object* str = alloc_string("updating non-placeholder binding");
		object* ls = alloc_list_3(str, name, value);
		return throw_error(cont, ls);
	}
	binding->data.binding.value = value;
	alloc_stack_reference(binding, value);
	
	// the environment is not the latest one, so it is discarded
	return call_discarding_cont(cont);
}

object bind_continued_proc;

object* bind_continued(object* args, object* cont) {
	object* environment;
	object* body;
	object* name;
	object* trace;
	delist_4(args, &environment, &body, &name, &trace);
	
	object* return_args = alloc_list_1(environment);
	object* return_call = alloc_call(&identity_proc, return_args, cont);
	object* return_cont = alloc_discarding_cont(return_call);
	
	object* update_args = alloc_list_2(name, environment);
	object* update_call = alloc_call(&update_binding_proc, update_args, return_cont);
	object* update_cont = alloc_cont(update_call);
	
	object* eval_args = alloc_list_3(body, environment, trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, update_cont);
	
	return perform_call(eval_call);
}

object bind_placeholder_proc;

object* bind_placeholder(object* args, object* cont) {
	object* environment;
	object* name;
	delist_2(args, &environment, &name);
	
	object* bind_args = alloc_list_3(placeholder_value(), name, environment);
	object* bind_call = alloc_call(&extend_environment_proc, bind_args, cont);
	
	return perform_call(bind_call);
}

object bind_placeholders_proc;

object* bind_placeholders(object* args, object* cont) {
	object* environment;
	object* names;
	delist_2(args, &environment, &names);
	
	if (is_empty_list(names)) {
		return call_cont(cont, environment);
	}
	else {
		object* next_args = alloc_list_1(list_rest(names));
		object* next_call = alloc_call(&bind_placeholders_proc, next_args, cont);
		object* next_cont = alloc_cont(next_call);
		
		object* bind_args = alloc_list_3(placeholder_value(), desyntax(list_first(names)), environment);
		object* bind_call = alloc_call(&extend_environment_proc, bind_args, next_cont);
		
		return perform_call(bind_call);
	}
}

object* define(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* name;
	object* body;
	delist_2(syntax, &name, &body);
	
	name = desyntax(name);
	
	// handles cases like (define ((f a) b) ..)
	if (is_list(name)) {
		object* desugared = alloc_list_3(syntax_procedure_obj(syntax_lambda), list_rest(name), body);
		object* new_syntax = alloc_list_2(list_first(name), desugared);
		
		object* call_args = alloc_list_3(new_syntax, environment, trace);
		object* call = alloc_call(syntax_procedure_obj(syntax_define), call_args, cont);
		
		return perform_call(call);
	}
	else {
		object* continued_args = alloc_list_3(body, name, trace);
		object* bind_continued_call = alloc_call(&bind_continued_proc, continued_args, cont);
		object* continued_cont = alloc_cont(bind_continued_call);
		
		object* bind_args = alloc_list_2(environment, name);
		object* bind_call = alloc_call(&bind_placeholder_proc, bind_args, continued_cont);
		
		return perform_call(bind_call);
	}
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
	
	alloc_stack_reference(delay, value);
	
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
		object* update_cont = alloc_cont(update_call);
		
		object* eval_call = alloc_call(&eval_proc, delay_value(obj), update_cont);
		
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
	object* force_cont = alloc_cont(force_call);
	
	object* eval_args = alloc_list_3(value, environment, trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, force_cont);
	
	return perform_call(eval_call);
}

object let_bind_proc;

object* let_bind(object* args, object* cont) {
	object* environment;
	object* bindings;
	object* trace;
	delist_3(args, &environment, &bindings, &trace);
	
	if (is_empty_list(bindings)) {
		return call_cont(cont, environment);
	}
	else {
		object* first = desyntax(list_first(bindings));
		object* name;
		object* value;
		delist_desyntax_2(first, &name, &value);
		
		if (!is_symbol(name)) {
			object* str = alloc_string("not a valid identifier");
			object* ls = alloc_list_2(str, name);
			return throw_trace_error(cont, trace, ls);
		}
		
		object* let_args = alloc_list_2(list_rest(bindings), trace);
		object* let_call = alloc_call(&let_bind_proc, let_args, cont);
		object* let_cont = alloc_cont(let_call);
		
		object* bind_args = alloc_list_2(name, environment);
		object* bind_call = alloc_call(&extend_environment_proc, bind_args, let_cont);
		object* bind_cont = alloc_cont(bind_call);
		
		object* eval_args = alloc_list_3(value, environment, trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, bind_cont);
		
		return perform_call(eval_call);
	}
}

object* let(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* bindings;
	object* body;
	delist_2(syntax, &bindings, &body);
	
	bindings = desyntax(bindings);
	
	object* eval_args = alloc_list_2(body, trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	object* eval_cont = alloc_cont(eval_call);
	
	object* let_args = alloc_list_3(environment, bindings, trace);
	object* let_call = alloc_call(&let_bind_proc, let_args, eval_cont);
	
	return perform_call(let_call);
}

object letrec_eval_single_proc;

object* letrec_eval_single(object* args, object* cont) {
	object* environment;
	object* bindings;
	object* trace;
	delist_3(args, &environment, &bindings, &trace);
	
	if (is_empty_list(bindings)) {
		return call_cont(cont, environment);
	}
	else {
		object* first = desyntax(list_first(bindings));
		object* name;
		object* value;
		delist_desyntax_2(first, &name, &value);
		
		object* next_args = alloc_list_3(environment, list_rest(bindings), trace);
		object* next_call = alloc_call(&letrec_eval_single_proc, next_args, cont);
		object* next_cont = alloc_discarding_cont(next_call);
		
		object* update_args = alloc_list_2(name, environment);
		object* update_call = alloc_call(&update_binding_proc, update_args, next_cont);
		object* update_cont = alloc_cont(update_call);
		
		object* eval_args = alloc_list_3(value, environment, trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, update_cont);
		
		return perform_call(eval_call);
	}
}

object letrec_bind_proc;

object* letrec_bind(object* args, object* cont) {
	object* names_values;
	object* environment;
	delist_2(args, &names_values, &environment);
	
	object* names = desyntax(list_first(names_values));
	
	object* dup = find_duplicate(names);
	if (!is_false(dup)) {
		return throw_error_string(cont, "duplicate binding");
	}
	else {
		object* bind_args = alloc_list_2(environment, names);
		object* bind_call = alloc_call(&bind_placeholders_proc, bind_args, cont);
		
		return perform_call(bind_call);
	}
}

object* letrec(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);

	object* bindings;
	object* body;
	delist_desyntax_2(syntax, &bindings, &body);
	
	object* eval_args = alloc_list_2(body, trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	object* eval_cont = alloc_cont(eval_call);
	
	object* eval_values_args = alloc_list_2(bindings, trace);
	object* eval_values_call = alloc_call(&letrec_eval_single_proc, eval_values_args, eval_cont);
	object* eval_values_cont = alloc_cont(eval_values_call);
	
	object* bind_args = alloc_list_1(environment);
	object* bind_call = alloc_call(&letrec_bind_proc, bind_args, eval_values_cont);
	object* bind_cont = alloc_cont(bind_call);
	
	object* unzip_args = alloc_list_1(bindings);
	object* unzip_call = alloc_call(&unzip_2_proc, unzip_args, bind_cont);
	
	return perform_call(unzip_call);
}

object rec_three_proc;

object* rec_three(object* args, object* cont) {
	object* environment;
	object* function;
	object* arguments;
	object* trace;
	delist_4(args, &environment, &function, &arguments, &trace);
	
	function->data.function.environment = environment;
	alloc_stack_reference(function, environment);
	
	object* eval_args = alloc_list_3(arguments, function, trace);
	object* eval_call = alloc_call(&eval_function_call_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object rec_two_proc;

object* rec_two(object* args, object* cont) {
	object* arguments;
	object* function;
	object* name;
	object* environment;
	object* trace;
	delist_5(args, &arguments, &function, &name, &environment, &trace);
	
	object* rec_args = alloc_list_3(function, arguments, trace);
	object* rec_call = alloc_call(&rec_three_proc, rec_args, cont);
	object* rec_cont = alloc_cont(rec_call);
	
	object* bind_args = alloc_list_3(function, name, environment);
	object* bind_call = alloc_call(&extend_environment_proc, bind_args, rec_cont);
	
	return perform_call(bind_call);
}

object rec_one_proc;

object* rec_one(object* args, object* cont) {
	object* pars_args;
	object* name;
	object* body;
	object* environment;
	object* trace;
	delist_5(args, &pars_args, &name, &body, &environment, &trace);
	
	object* parameters;
	object* arguments;
	delist_desyntax_2(pars_args, &parameters, &arguments);
	
	object* function = alloc_function(empty_environment(), parameters, body);
	
	object* rec_args = alloc_list_4(function, name, environment, trace);
	object* rec_call = alloc_call(&rec_two_proc, rec_args, cont);
	object* rec_cont = alloc_cont(rec_call);
	
	object* eval_args = alloc_list_3(arguments, environment, trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, rec_cont);
	
	return perform_call(eval_call);
}

object* rec(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* name;
	object* bindings;
	object* body;
	delist_3(syntax, &name, &bindings, &body);
	
	name = desyntax(name);
	bindings = desyntax(bindings);
	
	object* rec_args = alloc_list_4(name, body, environment, trace);
	object* rec_call = alloc_call(&rec_one_proc, rec_args, cont);
	object* rec_cont = alloc_cont(rec_call);
	
	object* unzip_args = alloc_list_1(bindings);
	object* unzip_call = alloc_call(&unzip_2_proc, unzip_args, rec_cont);
	
	return perform_call(unzip_call);
}

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
		object* curry_cont = alloc_cont(curry_call);
		
		object* reverse_args = alloc_list_1(parameters);
		object* reverse_call = alloc_call(&reverse_list_proc, reverse_args, curry_cont);
		
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
	
	object* eval_args = alloc_list_2(environment, trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, cont);
	object* eval_cont = alloc_cont(eval_call);

	object* curry_call = alloc_call(&start_curry_proc, empty_list(), eval_cont);
	object* curry_cont = alloc_cont(curry_call);
	
	object* eval_function_args = alloc_list_3(function, environment, trace);
	object* eval_function_call = alloc_call(&eval_proc, eval_function_args, curry_cont);
	
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
	
	//pick off the parameters left for the body lambda
	object* vs = values;
	object* ps = parameters;
	while (!is_empty_list(vs)) {
		vs = list_rest(vs);
		ps = list_rest(ps);
	}
	
	object* body = alloc_list_3(syntax_procedure_obj(syntax_lambda), ps, function_body(function));
	object* eval_args = alloc_list_2(body, trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	object* eval_cont = alloc_cont(eval_call);
	
	object* bind_args = alloc_list_3(values, parameters, environment);
	object* bind_call = alloc_call(&bind_values_proc, bind_args, eval_cont);
	
	return perform_call(bind_call);
}

object* apply(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* apply_args = alloc_list_2(environment, trace);
	object* apply_call = alloc_call(&start_apply_proc, apply_args, cont);
	object* apply_cont = alloc_cont(apply_call);
	
	object* eval_call = alloc_call(&eval_list_elements_proc, args, apply_cont);
	
	return perform_call(eval_call);
}

object eval_if_proc;

object* eval_if(object* args, object* cont) {
	object* condition;
	object* then;
	object* els;
	object* environment;
	object* trace;
	delist_5(args, &condition, &then, &els, &environment, &trace);
	
	object* ls = alloc_list_3(is_false(condition) ? els : then, environment, trace);
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
	object* next_cont = alloc_cont(next_call);
	
	object* call_args = alloc_list_3(condition, environment, trace);
	object* call = alloc_call(&eval_proc, call_args, next_cont);
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
		object* and_cont = alloc_cont(and_call);
		
		object* eval_args = alloc_list_3(list_first(elements), environment, trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, and_cont);
		
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
		object* and_cont = alloc_cont(and_call);
		
		object* eval_args = alloc_list_3(list_first(elements), environment, trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, and_cont);
		
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
		object* or_cont = alloc_cont(or_call);
		
		object* eval_args = alloc_list_3(list_first(elements), environment, trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, or_cont);
		
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
		object* or_cont = alloc_cont(or_call);
		
		object* eval_args = alloc_list_3(list_first(elements), environment, trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, or_cont);
		
		return perform_call(eval_call);
	}
}

object* struct_func(object* args, object* cont) {	
	object* call = alloc_call(&define_struct_proc, args, cont);
	
	return perform_call(call);
}

object syntax_procedure[syntax_count];

void add_syntax(char* name, static_syntax_procedure syntax, primitive_proc* proc) {
	init_syntax_procedure(&syntax_procedure[syntax], proc, syntax);
	add_static_binding(&syntax_procedure[syntax], name);
}

object* syntax_procedure_obj(static_syntax_procedure type) {
	return &syntax_procedure[type];
}

void init_base_syntax_procedures(void) {	
	add_syntax("define", syntax_define, &define);
	add_syntax("quote", syntax_quote, &quote);
	add_syntax("delay", syntax_delay, &delay);
	add_syntax("force", syntax_force, &force);
	add_syntax("let", syntax_let, &let);
	add_syntax("let-rec", syntax_letrec, &letrec);
	add_syntax("rec", syntax_rec, &rec);
	add_syntax("lambda", syntax_lambda, &lambda);
	add_syntax("curry", syntax_curry, &curry);
	add_syntax("apply", syntax_apply, &apply);
	add_syntax("if", syntax_if, &if_func);
	add_syntax("and", syntax_and, &and);
	add_syntax("or", syntax_or, &or);
	add_syntax("struct", syntax_struct, &struct_func);
	
	init_primitive(&update_delay, &update_delay_proc);
	init_primitive(&eval_force, &eval_force_proc);
	
	init_primitive(&let_bind, &let_bind_proc);

	init_primitive(&letrec_bind, &letrec_bind_proc);
	init_primitive(&letrec_eval_single, &letrec_eval_single_proc);
	
	init_primitive(&rec_one, &rec_one_proc);
	init_primitive(&rec_two, &rec_two_proc);
	init_primitive(&rec_three, &rec_three_proc);

	init_primitive(&bind_value, &bind_value_proc);
	init_primitive(&eval_if, &eval_if_proc);
	
	init_primitive(&eval_and, &eval_and_proc);
	init_primitive(&eval_or, &eval_or_proc);
	
	init_primitive(&bind_placeholder, &bind_placeholder_proc);
	init_primitive(&bind_placeholders, &bind_placeholders_proc);

	init_primitive(&bind_continued, &bind_continued_proc);
	init_primitive(&update_binding, &update_binding_proc);
	
	init_primitive(&start_curry, &start_curry_proc);
	init_primitive(&curry_one, &curry_one_proc);
	
	init_primitive(&start_apply, &start_apply_proc);
}
