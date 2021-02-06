#include "lists.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "list-util.h"
#include "call.h"
#include "eval.h"
#include "sequences.h"
#include "higher-order.h"
#include "syntax-base.h"

object* list(object* args, object* cont) {
	object* elements;
	object* environment;
	object* trace;
	delist_3(args, &elements, &environment, &trace);
	
	object* eval_args = alloc_list_4(elements, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object list_rest_proc;

object* list_seq_rest(object* args, object* cont) {
	object* ls;
	delist_1(args, &ls);
	
	return call_cont(cont, list_rest(ls));
}

object list_map_single_proc;

object* list_map_single(object* args, object* cont) {
	object* value;
	object* mapped;
	object* ls;
	object* function;
	object* trace;
	delist_5(args, &value, &mapped, &ls, &function, &trace);
	
	mapped = alloc_list_cell(value, mapped);
	
	if (is_empty_list(ls)) {
		return call_cont(cont, mapped);
	}
	else {
		object* next_args = alloc_list_4(mapped, list_rest(ls), function, trace);
		object* next_call = alloc_call(&list_map_single_proc, next_args, cont);
		
		object* eval_args = alloc_list_4(alloc_list_1(list_first(ls)), function, default_context(), trace);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, alloc_cont(next_call));
		
		return perform_call(eval_call);
	}
}

object* list_map(object* args, object* cont) {
	object* function;
	object* ls;
	object* trace;
	delist_3(args, &function, &ls, &trace);
	
	if (is_empty_list(ls)) {
		return call_cont(cont, ls);
	}
	else {
		object* reverse_call = alloc_call(&reverse_list_proc, empty_list(), cont);
		
		object* next_args = alloc_list_4(empty_list(), list_rest(ls), function, trace);
		object* next_call = alloc_call(&list_map_single_proc, next_args, alloc_cont(reverse_call));
		
		object* eval_args = alloc_list_4(alloc_list_1(list_first(ls)), function, default_context(), trace);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, alloc_cont(next_call));
		
		return perform_call(eval_call);
	}
}

object list_filter_single_proc;

object* list_filter_single(object* args, object* cont) {
	object* boolean;
	object* filtered;
	object* ls;
	object* function;
	object* trace;
	delist_5(args, &boolean, &filtered, &ls, &function, &trace);
	
	filtered = is_true(boolean) ? alloc_list_cell(list_first(ls), filtered) : filtered;
	ls = list_rest(ls);
	
	if (is_empty_list(ls)) {
		return call_cont(cont, filtered);
	}
	else {
		object* next_args = alloc_list_4(filtered, ls, function, trace);
		object* next_call = alloc_call(&list_filter_single_proc, next_args, cont);
		
		object* eval_args = alloc_list_4(alloc_list_1(list_first(ls)), function, default_context(), trace);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, alloc_cont(next_call));
		
		return perform_call(eval_call);
	}
}

object* list_filter(object* args, object* cont) {
	object* function;
	object* ls;
	object* trace;
	delist_3(args, &function, &ls, &trace);
	
	if (is_empty_list(ls)) {
		return call_cont(cont, ls);
	}
	else {
		object* reverse_call = alloc_call(&reverse_list_proc, empty_list(), cont);
		
		// create dummy object to simplify filter recursion
		ls = alloc_list_cell(no_object(), ls);
		
		object* next_args = alloc_list_5(false(), empty_list(), ls, function, trace);
		object* next_call = alloc_call(&list_filter_single_proc, next_args, alloc_cont(reverse_call));
		
		return perform_call(next_call);
	}
}

object list_fold_single_proc;

object* list_fold_single(object* args, object* cont) {
	object* value;
	object* ls;
	object* function;
	object* trace;
	delist_4(args, &value, &ls, &function, &trace);
	
	if (is_empty_list(ls)) {
		return call_cont(cont, value);
	}
	else {
		object* next_args = alloc_list_3(list_rest(ls), function, trace);
		object* next_call = alloc_call(&list_fold_single_proc, next_args, cont);
		
		object* function_args = alloc_list_2(value, list_first(ls));
		object* eval_args = alloc_list_4(function_args, function, default_context(), trace);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, alloc_cont(next_call));
		
		return perform_call(eval_call);
	}
}

object* list_fold(object* args, object* cont) {
	object* function;
	object* initial;
	object* ls;
	object* trace;
	delist_4(args, &function, &initial, &ls, &trace);
	
	object* next_args = alloc_list_4(initial, ls, function, trace);
	object* next_call = alloc_call(&list_fold_single_proc, next_args, cont);
	
	return perform_call(next_call);
}

object* fold_as_list(object* args, object* cont) {
	object* ls;
	object* function;
	object* initial;
	object* trace;
	delist_4(args, &ls, &function, &initial, &trace);
	
	object* next_args = alloc_list_4(function, initial, ls, trace);
	object* next_call = alloc_call(&list_fold_proc, next_args, cont);
	
	return perform_call(next_call);
}

void init_list_procedures(void) {
	add_syntax("list", syntax_list, context_value, &list);
	
	add_first_procedure(type_list, &list_first);
	init_primitive(&list_seq_rest, &list_rest_proc);
	add_rest_procedure(type_list, &list_rest_proc);
	
	init_primitive(&list_map, &list_map_proc);
	init_primitive(&list_map_single, &list_map_single_proc);
	add_map_procedure(type_list, &list_map_proc);
	
	init_primitive(&list_filter, &list_filter_proc);
	init_primitive(&list_filter_single, &list_filter_single_proc);
	add_filter_procedure(type_list, &list_filter_proc);
	
	init_primitive(&list_fold, &list_fold_proc);
	init_primitive(&fold_as_list, &fold_as_list_proc);
	init_primitive(&list_fold_single, &list_fold_single_proc);
	add_fold_procedure(type_list, &list_fold_proc);
}
