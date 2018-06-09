#include "eval.h"

#include <stdlib.h>
#include <string.h>
#include "data-structures.h"
#include "object-init.h"
#include "global-variables.h"
#include "environments.h"
#include "call.h"
#include "delist.h"
#include "list-util.h"

object eval_identity_proc;
object eval_symbol_proc;
object eval_list_proc;
object eval_list_rest_proc;

object* eval_identity(object* args, object* cont) {
	object* obj;
	object* environment;
	object* trace;
	delist_3(args, &obj, &environment, &trace);
	
	obj = desyntax(obj);
	
	return call_cont(cont, obj);
}

object eval_syntax_proc;

object* eval_syntax(object* args, object* cont) {
	object* syntax;
	object* rest;
	object* environment;
	object* trace;
	delist_4(args, &syntax, &rest, &environment, &trace);
	
	object* ls = alloc_list_3(rest, environment, trace);
	object* call = alloc_call(syntax, ls, cont);
	
	return perform_call(call);
}

object eval_primitive_procedure_call_proc;

object* eval_primitive_procedure_call(object* args, object* cont) {
	object* arguments;
	object* proc;
	object* environment;
	object* trace;
	delist_4(args, &arguments, &proc, &environment, &trace);
	
	object* call = alloc_call(proc, arguments, cont);
	
	return perform_call(call);
}

object eval_primitive_procedure_proc;

object* eval_primitive_procedure(object* args, object* cont) {
	object* proc;
	object* arguments;
	object* environment;
	object* trace;
	delist_4(args, &proc, &arguments, &environment, &trace);
	
	object* proc_ls = alloc_list_3(proc, environment, trace);
	object* proc_call = alloc_call(&eval_primitive_procedure_call_proc, proc_ls, cont);
	object* proc_cont = alloc_cont(proc_call);
	
	object* ls = alloc_list_3(arguments, environment, trace);
	object* call = alloc_call(&eval_list_elements_proc, ls, proc_cont);
	
	return perform_call(call);
}

object* eval_function_call(object* args, object* cont) {
	object* arguments;
	object* function;
	object* trace;
	delist_3(args, &arguments, &function, &trace);
	
	function = desyntax(function);
	
	object* eval_args = alloc_list_2(function_body(function), trace);
	object* eval_call = alloc_call(&eval_with_environment_proc, eval_args, cont);
	object* eval_cont = alloc_cont(eval_call);
	
	object* bind_args = alloc_list_3(arguments, function_parameters(function), function_environment(function));
	object* bind_call = alloc_call(&bind_values_proc, bind_args, eval_cont);
	
	return perform_call(bind_call);
}

object eval_function_proc;

object* eval_function(object* args, object* cont) {
	object* function;
	object* arguments;
	object* environment;
	object* trace;
	delist_4(args, &function, &arguments, &environment, &trace);
	
	object* call_args = alloc_list_2(function, trace);
	object* call = alloc_call(&eval_function_call_proc, call_args, cont);
	object* next_cont = alloc_cont(call);
	
	object* eval_args = alloc_list_3(arguments, environment, trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, next_cont);
	
	return perform_call(eval_call);
}

object eval_list_elements_rest_proc;

object* eval_list_elements_rest(object* args, object* cont) {
	object* last;
	object* unevaled;
	object* environment;
	object* trace;
	delist_4(args, &last, &unevaled, &environment, &trace);
	
	if (is_empty_list(unevaled)) {
		return call_discarding_cont(cont);
	}
	else {
		object* build_args = alloc_list_3(list_rest(unevaled), environment, trace);
		object* build_call = alloc_call(&eval_list_elements_rest_proc, build_args, cont);
		object* build_cont = alloc_cont(build_call);

		object* add_args = alloc_list_1(last);
		object* add_call = alloc_call(&add_to_list_proc, add_args, build_cont);
		object* add_cont = alloc_cont(add_call);
		
		object* eval_args = alloc_list_3(list_first(unevaled), environment, trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, add_cont);
		return perform_call(eval_call);
	}
}

object eval_list_elements_first_proc;

object* eval_list_elements_first(object* args, object* cont) {
	object* elements;
	object* environment;
	object* trace;
	delist_3(args, &elements, &environment, &trace);
	
	object* rest_args = alloc_list_3(list_rest(elements), environment, trace);
	
	object* make_args = alloc_list_2(&eval_list_elements_rest_proc, rest_args);
	object* make_call = alloc_call(&make_list_proc, make_args, cont);
	object* make_cont = alloc_cont(make_call);
	
	object* eval_args = alloc_list_3(list_first(elements), environment, trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, make_cont);
	
	return perform_call(eval_call);
}

object* eval_list_elements(object* args, object* cont) {
	object* elements;
	object* environment;	
	object* trace;
	delist_3(args, &elements, &environment, &trace);
	
	elements = desyntax(elements);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, empty_list());
	}
	else {
		object* call_args = alloc_list_3(elements, environment, trace);
		object* call = alloc_call(&eval_list_elements_first_proc, call_args, cont);
		
		return perform_call(call);
	}
}

object* eval_list_rest(object* args, object* cont) {
	object* first;
	object* rest;
	object* environment;
	object* trace;
	delist_4(args, &first, &rest, &environment, &trace);
	
	object* proc;
	
	switch (first->type) {
		case type_syntax_procedure:
			proc = &eval_syntax_proc;
			break;
		case type_primitive_procedure:
			proc = &eval_primitive_procedure_proc;
			break;
		case type_function:
			proc = &eval_function_proc;
			break;
		default: {
			object* str = alloc_string("application of non-function");
			object* ls = alloc_list_3(str, first, rest);
			return throw_error(cont, ls);
		}
	}
	
	object* call = alloc_call(proc, args, cont);
	return perform_call(call);
}

object error_trace_or_continue_proc;

object* error_trace_or_continue(object* args, object* cont) {
	object* obj;
	object* trace;
	delist_2(args, &obj, &trace);
	
	if (is_internal_error(obj) && is_false(internal_error_trace(obj))) {
		obj = alloc_internal_error(trace, internal_error_message(obj));
	}
	return call_cont(cont, obj);
}

object* eval_list(object* args, object* cont) {
	object* stx;
	object* environment;
	object* trace;
	delist_3(args, &stx, &environment, &trace);
	
	object* list = desyntax(stx);
	
	if (is_empty_list(list)) {
		return throw_error_string(cont, "eval of empty list");
	}
	else {
		object* next_args = alloc_list_3(list_rest(list), environment, trace);
		object* next_call = alloc_call(&eval_list_rest_proc, next_args, cont);
		object* next_cont = alloc_cont(next_call);
		
		object* call_args = alloc_list_3(list_first(list), environment, trace);
		object* call = alloc_call(&eval_proc, call_args, next_cont);
		
		return perform_call(call);
	}
}

object* eval_symbol(object* args, object* cont) {
	object* stx;
	object* environment;
	object* trace;
	delist_3(args, &stx, &environment, &trace);
	
	object* obj = desyntax(stx);
	
	object* binding = find_in_environment(environment, obj, 0);
	
	if (is_no_binding(binding)) {
		object* str = alloc_string("unbound variable");
		object* ls = alloc_list_2(str, obj);
		return throw_error(cont, ls);
	}
	else {
		object* value = binding_value(binding);
		if (is_struct_definition(value)) {
			value = struct_definition_constructor(value);
		}
		return call_cont(cont, value);
	}
}

object* eval_with_environment(object* args, object* cont) {
	object* environment;
	object* body;
	object* trace;
	delist_3(args, &environment, &body, &trace);
	
	object* ls = alloc_list_3(body, environment, trace);
	object* call = alloc_call(&eval_proc, ls, cont);
	
	return perform_call(call);
}

object* eval(object* args, object* cont) {
	object* stx;
	object* environment;
	object* trace;
	delist_3(args, &stx, &environment, &trace);
	
	trace = alloc_trace_list(stx, trace);
	
	object* trace_args = alloc_list_1(trace);
	object* trace_call = alloc_call(&error_trace_or_continue_proc, trace_args, cont);
	object* trace_cont = alloc_catching_cont(trace_call);
	
	object* obj = desyntax(stx);
	
	object* proc;
	
	switch (obj->type) {
		case type_symbol:
			proc = &eval_symbol_proc;
			break;
		case type_list:
			proc = &eval_list_proc;
			break;
		default:
			proc = &eval_identity_proc;
			break;
	}
	
	object* call_args = alloc_list_3(obj, environment, trace);
	object* call = alloc_call(proc, call_args, trace_cont);
	
	return perform_call(call);
}

void init_eval_procedures(void) {
	init_primitive(&eval, &eval_proc);
	init_primitive(&eval_with_environment, &eval_with_environment_proc);
	init_primitive(&eval_identity, &eval_identity_proc);
	init_primitive(&eval_symbol, &eval_symbol_proc);
	init_primitive(&eval_list, &eval_list_proc);
	init_primitive(&eval_list_rest, &eval_list_rest_proc);
	init_primitive(&eval_syntax, &eval_syntax_proc);
	init_primitive(&eval_primitive_procedure, &eval_primitive_procedure_proc);
	init_primitive(&eval_primitive_procedure_call, &eval_primitive_procedure_call_proc);
	
	init_primitive(&eval_function, &eval_function_proc);
	init_primitive(&eval_function_call, &eval_function_call_proc);
	
	init_primitive(&eval_list_elements, &eval_list_elements_proc);
	init_primitive(&eval_list_elements_first, &eval_list_elements_first_proc);
	init_primitive(&eval_list_elements_rest, &eval_list_elements_rest_proc);
	
	init_primitive(&error_trace_or_continue, &error_trace_or_continue_proc);
}
