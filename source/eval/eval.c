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
#include "string-util.h"

object* object_eval_procedure[type_count];

void add_object_eval_procedure(object_type type, object* proc) {
	object_eval_procedure[type] = proc;
}

object* list_application_procedure[type_count];

void add_list_application_procedure(object_type type, object* proc) {
	list_application_procedure[type] = proc;
}

object eval_identity_proc;
object eval_symbol_proc;

object* eval_identity(object* args, object* cont) {
	object* obj;
	object* environment;
	object* context;
	object* trace;
	delist_4(args, &obj, &environment, &context, &trace);
	
	obj = desyntax(obj);
	
	return call_cont(cont, obj);
}

object eval_list_elements_next_proc;

object* eval_list_elements_next(object* args, object* cont) {
	object* evaled;
	object* elements;
	object* environment;
	object* context;
	object* trace;
	delist_5(args, &evaled, &elements, &environment, &context, &trace);
	
	if (is_empty_list(elements)) {
		return call_cont(cont, evaled);
	}
	else {
		object* next_args = alloc_list_4(list_rest(elements), environment, context, trace);
		object* next_call = alloc_call(&eval_list_elements_next_proc, next_args, cont);
		
		object* link_call = alloc_call(&link_list_proc, alloc_list_1(evaled), alloc_cont(next_call));
		
		object* eval_args = alloc_list_4(list_first(elements), environment, context, trace);
		object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(link_call));
		
		return perform_call(eval_call);
	}
}

object* eval_list_elements(object* args, object* cont) {
	object* elements;
	object* environment;
	object* context;
	object* trace;
	delist_4(args, &elements, &environment, &context, &trace);
	
	object* reverse_call = alloc_call(&reverse_list_proc, empty_list(), cont);
	
	object* next_args = alloc_list_5(empty_list(), desyntax(elements), environment, context, trace);
	object* next_call = alloc_call(&eval_list_elements_next_proc, next_args, alloc_cont(reverse_call));
	
	return perform_call(next_call);
}

object list_application_undefined_proc;

object* list_application_undefined(object* args, object* cont) {
	object* first;
	object* rest;
	object* environment;
	object* context;
	object* trace;
	delist_5(args, &first, &rest, &environment, &context, &trace);
	
	object* str = alloc_string_append_2(alloc_string("application of "), alloc_string(object_type_name(first)));
	object* ls = alloc_list_3(str, first, rest);
	return throw_error(cont, ls);
}

object eval_list_application_rest_proc;

object* eval_list_application_rest(object* args, object* cont) {
	object* first;
	object* rest;
	object* environment;
	object* context;
	object* trace;
	delist_5(args, &first, &rest, &environment, &context, &trace);
	
	object* proc = list_application_procedure[first->type];
	
	object* call = alloc_call(proc, args, cont);
	return perform_call(call);
}

object eval_list_application_proc;

object* eval_list_application(object* args, object* cont) {
	object* stx;
	object* environment;
	object* context;
	object* trace;
	delist_4(args, &stx, &environment, &context, &trace);
	
	object* list = desyntax(stx);
	
	if (is_empty_list(list)) {
		return throw_error_string(cont, "eval of empty list");
	}
	else {
		object* next_args = alloc_list_4(list_rest(list), environment, context, trace);
		object* next_call = alloc_call(&eval_list_application_rest_proc, next_args, cont);
		
		object* call_args = alloc_list_4(list_first(list), environment, context, trace);
		object* call = alloc_call(&eval_proc, call_args, alloc_cont(next_call));
		
		return perform_call(call);
	}
}

object* eval_symbol(object* args, object* cont) {
	object* stx;
	object* environment;
	object* context;
	object* trace;
	delist_4(args, &stx, &environment, &context, &trace);
	
	object* obj = desyntax(stx);
	
	object* binding = find_in_environment(environment, obj, 0);
	
	if (is_no_binding(binding)) {
		object* str = alloc_string("unbound variable");
		object* ls = alloc_list_2(str, obj);
		return throw_error(cont, ls);
	}
	else {
		object* value = binding_value(binding);
		return call_cont(cont, value);
	}
}

object eval_binding_error_proc;

object* eval_binding_error(object* args, object* cont) {
	object* stx;
	object* environment;
	object* context;
	object* trace;
	delist_4(args, &stx, &environment, &context, &trace);
	
	return throw_error_string(cont, "binding in eval context");
}

object* eval_with_environment(object* args, object* cont) {
	object* environment;
	object* body;
	object* context;
	object* trace;
	delist_4(args, &environment, &body, &context, &trace);
	
	object* ls = alloc_list_4(body, environment, context, trace);
	object* call = alloc_call(&eval_proc, ls, cont);
	
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

object* eval(object* args, object* cont) {
	object* stx;
	object* environment;
	object* context;
	object* trace;
	delist_4(args, &stx, &environment, &context, &trace);
	
	trace = alloc_trace_list(stx, trace);
	
	object* trace_args = alloc_list_1(trace);
	object* trace_call = alloc_call(&error_trace_or_continue_proc, trace_args, cont);
	object* trace_cont = alloc_catching_cont(trace_call);
	
	object* obj = desyntax(stx);
	
	object* proc = object_eval_procedure[obj->type];
	
	object* call_args = alloc_list_4(obj, environment, context, trace);
	object* call = alloc_call(proc, call_args, trace_cont);
	
	return perform_call(call);
}

void init_eval_procedures(void) {
	init_primitive(&eval, &eval_proc);
	init_primitive(&eval_with_environment, &eval_with_environment_proc);
	init_primitive(&eval_identity, &eval_identity_proc);
	init_primitive(&eval_symbol, &eval_symbol_proc);
	init_primitive(&eval_list_application, &eval_list_application_proc);
	init_primitive(&eval_list_application_rest, &eval_list_application_rest_proc);
	init_primitive(&eval_binding_error, &eval_binding_error_proc);
	
	init_primitive(&list_application_undefined, &list_application_undefined_proc);
	
	init_primitive(&eval_list_elements, &eval_list_elements_proc);
	init_primitive(&eval_list_elements_next, &eval_list_elements_next_proc);
	
	init_primitive(&error_trace_or_continue, &error_trace_or_continue_proc);
	
	for (object_type k = type_none; k < type_count; k++) {
		add_object_eval_procedure(k, &eval_identity_proc);
		add_list_application_procedure(k, &list_application_undefined_proc);
	}
	
	add_object_eval_procedure(type_symbol, &eval_symbol_proc);
	add_object_eval_procedure(type_list, &eval_list_application_proc);
	add_object_eval_procedure(type_binding, &eval_binding_error_proc);
}
