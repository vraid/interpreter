#include "syntax-base.h"

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
#include "repl-top.h"
#include "print.h"

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

object update_binding_proc;

object* update_binding(object* args, object* cont) {
	object* value;
	object* binding;
	delist_2(args, &value, &binding);
	
	if (is_no_binding(binding) || !is_placeholder_value(binding_value(binding))) {
		object* str = alloc_string("updating non-placeholder binding");
		object* ls = alloc_list_3(str, binding_name(binding), value);
		return throw_error(cont, ls);
	}
	binding->data.binding.value = value;
	alloc_mutation_reference(binding, value);
	
	return call_discarding_cont(cont);
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
	environment = alloc_list_cell(binding, environment);
	
	object* return_args = alloc_list_1(environment);
	object* return_call = alloc_call(&identity_proc, return_args, cont);
	object* return_cont = alloc_discarding_cont(return_call);
	
	object* update_args = alloc_list_1(binding);
	object* update_call = alloc_call(&update_binding_proc, update_args, return_cont);
	
	object* eval_args = alloc_list_4(environment, body, default_context(), trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, alloc_cont(update_call));
	
	return perform_call(eval_call);
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
		
		object* binding = alloc_placeholder_binding(name);
		object* new_env = alloc_list_cell(binding, environment);
		
		object* let_args = alloc_list_3(new_env, list_rest(bindings), trace);
		object* let_call = alloc_call(&let_bind_proc, let_args, cont);
		object* let_cont = alloc_discarding_cont(let_call);
		
		object* update_args = alloc_list_1(binding);
		object* update_call = alloc_call(&update_binding_proc, update_args, let_cont);
		
		object* eval_args = alloc_list_4(environment, value, default_context(), trace);
		object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, alloc_cont(update_call));
		
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
	
	object* eval_args = alloc_list_3(body, scope_context(), trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	object* let_args = alloc_list_3(environment, bindings, trace);
	object* let_call = alloc_call(&let_bind_proc, let_args, alloc_cont(eval_call));
	
	return perform_call(let_call);
}

object letrec_eval_proc;

object* letrec_eval(object* args, object* cont) {
	object* updates;
	object* environment;
	object* trace;
	delist_3(args, &updates, &environment, &trace);
	
	if (is_empty_list(updates)) {
		return call_cont(cont, environment);
	}
	else {
		object* binding;
		object* value;
		delist_2(list_first(updates), &binding, &value);
		
		object* next_args = alloc_list_3(list_rest(updates), environment, trace);
		object* next_call = alloc_call(&letrec_eval_proc, next_args, cont);
		object* next_cont = alloc_discarding_cont(next_call);
		
		object* update_args = alloc_list_1(binding);
		object* update_call = alloc_call(&update_binding_proc, update_args, next_cont);
		
		object* eval_args = alloc_list_4(environment, value, default_context(), trace);
		object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, alloc_cont(update_call));
		
		return perform_call(eval_call);
	}
}

object letrec_bind_proc;

object* letrec_bind(object* args, object* cont) {
	object* environment;
	object* bindings;
	object* trace;
	delist_3(args, &environment, &bindings, &trace);
	
	bindings = desyntax(bindings);
	
	object* env = environment;
	object* names = empty_list();
	object* updates = empty_list();
	object* name;
	object* value;
	
	while (!is_empty_list(bindings)) {
		delist_2(desyntax(list_first(bindings)), &name, &value);
		name = desyntax(name);
		names = alloc_list_cell(name, names);
		
		object* binding = alloc_placeholder_binding(name);
		env = alloc_list_cell(binding, env);
		updates = alloc_list_cell(alloc_list_2(binding, value), updates);
		
		bindings = list_rest(bindings);
	}
	
	object* dup = find_duplicate(names);
	if (!is_false(dup)) {
		return throw_error_string(cont, "duplicate binding");
	}
	
	object* eval_args = alloc_list_2(env, trace);
	object* eval_call = alloc_call(&letrec_eval_proc, eval_args, cont);
	
	object* reverse_args = alloc_list_1(updates);
	object* reverse_call = alloc_call(&reverse_list_proc, reverse_args, alloc_cont(eval_call));
	
	return perform_call(reverse_call);
}

object* letrec(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* bindings;
	object* body;
	delist_desyntax_2(syntax, &bindings, &body);
	
	object* eval_args = alloc_list_3(body, scope_context(), trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	object* bind_args = alloc_list_3(environment, bindings, trace);
	object* bind_call = alloc_call(&letrec_bind_proc, bind_args, alloc_cont(eval_call));
	
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
	
	object* binding = alloc_placeholder_binding(name);
	environment = alloc_list_cell(binding, environment);
	
	object* parameters;
	object* arguments;
	delist_desyntax_2(pars_args, &parameters, &arguments);
	
	object* eval_args = alloc_list_4(environment, alloc_list_cell(name, arguments), default_context(), trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	object* eval_cont = alloc_discarding_cont(eval_call);
	
	object* update_args = alloc_list_1(binding);
	object* update_call = alloc_call(&update_binding_proc, update_args, eval_cont);
	
	object* function_syntax = alloc_list_2(parameters, body);
	object* define_args = alloc_list_3(function_syntax, environment, trace);
	object* define_call = alloc_call(syntax_procedure_obj(syntax_lambda), define_args, alloc_cont(update_call));
	
	return perform_call(define_call);
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
	
	object* unzip_args = alloc_list_1(bindings);
	object* unzip_call = alloc_call(&unzip_2_proc, unzip_args, alloc_cont(rec_call));
	
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
	
	//pick off the parameters left for the body lambda
	object* vs = values;
	object* ps = parameters;
	while (!is_empty_list(vs)) {
		vs = list_rest(vs);
		ps = list_rest(ps);
	}
	
	object* body = alloc_list_3(syntax_procedure_obj(syntax_lambda), ps, function_body(function));
	object* eval_args = alloc_list_3(body, default_context(), trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	
	object* bind_args = alloc_list_3(values, parameters, environment);
	object* bind_call = alloc_call(&bind_values_proc, bind_args, alloc_cont(eval_call));
	
	return perform_call(bind_call);
}

object* apply(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* apply_args = alloc_list_3(environment, default_context(), trace);
	object* apply_call = alloc_call(&start_apply_proc, apply_args, cont);
	
	object* eval_call = alloc_call(&eval_list_elements_proc, args, alloc_cont(apply_call));
	
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
	
	object* next_args = alloc_list_5(then, els, environment, default_context(), trace);
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
		object* and_args = alloc_list_4(list_rest(elements), environment, default_context(), trace);
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
		object* and_args = alloc_list_4(list_rest(elements), environment, default_context(), trace);
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
		object* or_args = alloc_list_4(list_rest(elements), environment, default_context(), trace);
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
		object* or_args = alloc_list_4(list_rest(elements), environment, default_context(), trace);
		object* or_call = alloc_call(&eval_or_proc, or_args, cont);
		
		object* eval_args = alloc_list_4(list_first(elements), environment, default_context(), trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(or_call));
		
		return perform_call(eval_call);
	}
}

object* struct_func(object* args, object* cont) {	
	object* call = alloc_call(&define_struct_proc, args, cont);
	
	return perform_call(call);
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

void add_syntax(char* name, static_syntax_procedure syntax, context_type context, primitive_proc* proc) {
	init_syntax_procedure(&syntax_procedure[syntax], proc, syntax);
	set_syntax_properties(syntax, name, context);
	add_static_binding(name, &syntax_procedure[syntax]);
}

object* syntax_procedure_obj(static_syntax_procedure type) {
	return &syntax_procedure[type];
}

void init_base_syntax_procedures(void) {
	add_syntax("define", syntax_define, context_repl, &define);
	add_syntax("struct", syntax_struct, context_repl, &struct_func);
	add_syntax("quote", syntax_quote, context_value, &quote);
	add_syntax("delay", syntax_delay, context_value, &delay);
	add_syntax("force", syntax_force, context_value, &force);
	add_syntax("let", syntax_let, context_value, &let);
	add_syntax("letrec", syntax_letrec, context_value, &letrec);
	add_syntax("rec", syntax_rec, context_value, &rec);
	add_syntax("lambda", syntax_lambda, context_value, &lambda);
	add_syntax("curry", syntax_curry, context_value, &curry);
	add_syntax("apply", syntax_apply, context_value, &apply);
	add_syntax("if", syntax_if, context_value, &if_func);
	add_syntax("and", syntax_and, context_value, &and);
	add_syntax("or", syntax_or, context_value, &or);
	
	init_syntax_procedure(&syntax_procedure[syntax_enter_scope], &enter_scope, syntax_enter_scope);
	set_syntax_properties(syntax_enter_scope, "enter-scope", context_scope);
	init_syntax_procedure(&syntax_procedure[syntax_rewind_scope], &rewind_scope, syntax_rewind_scope);
	set_syntax_properties(syntax_rewind_scope, "rewind-scope", context_repl);
	
	init_primitive(&update_delay, &update_delay_proc);
	init_primitive(&eval_force, &eval_force_proc);
	
	init_primitive(&let_bind, &let_bind_proc);
	
	init_primitive(&letrec_eval, &letrec_eval_proc);
	init_primitive(&letrec_bind, &letrec_bind_proc);
	
	init_primitive(&rec_one, &rec_one_proc);

	init_primitive(&eval_if, &eval_if_proc);
	
	init_primitive(&eval_and, &eval_and_proc);
	init_primitive(&eval_or, &eval_or_proc);
	
	init_primitive(&update_binding, &update_binding_proc);
	
	init_primitive(&start_curry, &start_curry_proc);
	init_primitive(&curry_one, &curry_one_proc);
	
	init_primitive(&start_apply, &start_apply_proc);
}
