#include "let-syntax.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "list-util.h"
#include "syntax-base.h"
#include "memory-handling.h"
#include "environments.h"
#include "call.h"
#include "eval.h"

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

object* let(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* bindings;
	object* body;
	delist_2(syntax, &bindings, &body);
	
	bindings = desyntax(bindings);
	
	object* new_environment = alloc_list_cell(empty_list(), environment);
	
	object* eval_args = alloc_list_4(body, new_environment, scope_context(), trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, cont);
	
	object* next_cont = alloc_discarding_cont(eval_call);
	object* call = eval_call;
	
	object* new_bindings = empty_list();
	object* updates = empty_list();
	
	// allocate placeholders
	while (!is_empty_list(bindings)) {
		object* first = desyntax(list_first(bindings));
		object* name = desyntax(binding_name(first));
		object* value = desyntax(binding_value(first));
		
		if (!is_symbol(name)) {
			object* str = alloc_string("not a valid identifier");
			object* ls = alloc_list_2(str, name);
			return throw_trace_error(cont, trace, ls);
		}
		
		object* binding = alloc_placeholder_binding(name);
		object* new_env = alloc_list_cell(new_bindings, environment);
		updates = alloc_list_cell(alloc_list_3(binding, value, new_env), updates);
		
		new_bindings = alloc_list_cell(binding, new_bindings);
		bindings = list_rest(bindings);
	}
	// update bindings in correct order
	while (!is_empty_list(updates)) {
		object* binding;
		object* value;
		object* new_env;
		delist_3(list_first(updates), &binding, &value, &new_env);
		
		object* update_args = alloc_list_1(binding);
		object* update_call = alloc_call(&update_binding_proc, update_args, next_cont);
		object* update_cont = alloc_cont(update_call);
		
		object* eval_binding_args = alloc_list_4(value, new_env, scope_context(), trace);
		call = alloc_call(&eval_proc, eval_binding_args, update_cont);
		next_cont = alloc_discarding_cont(call);
		
		updates = list_rest(updates);
	}
	
	init_list_cell(new_environment, new_bindings, environment);
	
	return perform_call(call);
}

object* letrec(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* bindings;
	object* body;
	delist_2(syntax, &bindings, &body);
	
	bindings = desyntax(bindings);
	
	object* new_environment = alloc_list_cell(empty_list(), environment);
	
	object* eval_args = alloc_list_4(body, new_environment, scope_context(), trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, cont);
	
	object* next_cont = alloc_discarding_cont(eval_call);
	object* call = eval_call;
	
	object* names = empty_list();
	object* new_bindings = empty_list();
	object* updates = empty_list();
	
	// allocate placeholders
	while (!is_empty_list(bindings)) {
		object* first = desyntax(list_first(bindings));
		object* name = desyntax(binding_name(first));
		object* value = desyntax(binding_value(first));
		
		if (!is_symbol(name)) {
			object* str = alloc_string("not a valid identifier");
			object* ls = alloc_list_2(str, name);
			return throw_trace_error(cont, trace, ls);
		}
		
		names = alloc_list_cell(name, names);
		object* binding = alloc_placeholder_binding(name);
		updates = alloc_list_cell(alloc_list_2(binding, value), updates);
		
		new_bindings = alloc_list_cell(binding, new_bindings);
		bindings = list_rest(bindings);
	}
	
	object* dup = find_duplicate(names);
	if (!is_false(dup)) {
		return throw_error_string(cont, "duplicate binding");
	}
	
	init_list_cell(new_environment, new_bindings, environment);
	
	// update bindings in correct order
	while (!is_empty_list(updates)) {
		object* binding;
		object* value;
		delist_2(list_first(updates), &binding, &value);
		
		object* update_args = alloc_list_1(binding);
		object* update_call = alloc_call(&update_binding_proc, update_args, next_cont);
		
		object* eval_binding_args = alloc_list_4(value, new_environment, scope_context(), trace);
		call = alloc_call(&eval_proc, eval_binding_args, alloc_cont(update_call));
		next_cont = alloc_discarding_cont(call);
		
		updates = list_rest(updates);
	}
	
	return perform_call(call);
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
	
	object* binding = alloc_placeholder_binding(desyntax(name));
	environment = alloc_list_cell(alloc_list_1(binding), environment);
	
	bindings = desyntax(bindings);
	object* rev_bindings = empty_list();
	while (!is_empty_list(bindings)) {
		rev_bindings = alloc_list_cell(list_first(bindings), rev_bindings);
		bindings = list_rest(bindings);
	}
	
	object* parameters = empty_list();
	object* arguments = empty_list();
	
	while (!is_empty_list(rev_bindings)) {
		object* first = desyntax(list_first(rev_bindings));
		object* par = desyntax(binding_name(first));
		object* arg = desyntax(binding_value(first));
		
		parameters = alloc_list_cell(par, parameters);
		arguments = alloc_list_cell(arg, arguments);
		rev_bindings = list_rest(rev_bindings);
	}
	
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

void init_let_syntax_procedures(void) {
	add_syntax("let", syntax_let, context_value, &let);
	add_syntax("letrec", syntax_letrec, context_value, &letrec);
	add_syntax("rec", syntax_rec, context_value, &rec);
	
	init_primitive(&update_binding, &update_binding_proc);
}
