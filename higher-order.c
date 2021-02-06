#include "higher-order.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "list-util.h"
#include "string-util.h"
#include "call.h"
#include "eval.h"
#include "syntax-base.h"

object* sequence_map_procedure[type_count];
object* sequence_filter_procedure[type_count];
object* sequence_fold_procedure[type_count];

void add_map_procedure(object_type type, object* proc) {
	sequence_map_procedure[type] = proc;
}

void add_filter_procedure(object_type type, object* proc) {
	sequence_filter_procedure[type] = proc;
}

void add_fold_procedure(object_type type, object* proc) {
	sequence_fold_procedure[type] = proc;
}

object map_undefined_proc;

object* map_undefined(object* args, object* cont) {
	object* function;
	object* sequence;
	object* trace;
	delist_3(args, &function, &sequence, &trace);
	
	object* str = alloc_string_append_2(alloc_string("map not defined on type: "), alloc_string(object_type_name(sequence)));
	return throw_error(cont, str);
}

object* map(object* args, object* cont) {
	object* syntax_procedure;
	object* trace;
	delist_2(args, &syntax_procedure, &trace);
	
	object* function;
	object* sequence;
	delist_2(syntax_procedure, &function, &sequence);
	
	object* proc = sequence_map_procedure[sequence->type];
	object* call_args = alloc_list_3(function, sequence, trace);
	object* call = alloc_call(proc, call_args, cont);
	
	return perform_call(call);
}

object* eval_and_map(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* map_args = alloc_list_1(trace);
	object* map_call = alloc_call(&map_proc, map_args, cont);
	
	object* eval_args = alloc_list_4(syntax, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, alloc_cont(map_call));
	
	return perform_call(eval_call);
}

object filter_undefined_proc;

object* filter_undefined(object* args, object* cont) {
	object* function;
	object* sequence;
	object* trace;
	delist_3(args, &function, &sequence, &trace);
	
	object* str = alloc_string_append_2(alloc_string("filter not defined on type: "), alloc_string(object_type_name(sequence)));
	return throw_error(cont, str);
}

object* filter(object* args, object* cont) {
	object* syntax_procedure;
	object* trace;
	delist_2(args, &syntax_procedure, &trace);
	
	object* function;
	object* sequence;
	delist_2(syntax_procedure, &function, &sequence);
	
	object* proc = sequence_filter_procedure[sequence->type];
	object* call_args = alloc_list_3(function, sequence, trace);
	object* call = alloc_call(proc, call_args, cont);
	
	return perform_call(call);
}

object* eval_and_filter(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* filter_args = alloc_list_1(trace);
	object* filter_call = alloc_call(&filter_proc, filter_args, cont);
	
	object* eval_args = alloc_list_4(syntax, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, alloc_cont(filter_call));
	
	return perform_call(eval_call);
}

object fold_undefined_proc;

object* fold_undefined(object* args, object* cont) {
	object* function;
	object* initial;
	object* sequence;
	object* trace;
	delist_4(args, &function, &initial, &sequence, &trace);
	
	object* str = alloc_string_append_2(alloc_string("fold not defined on type: "), alloc_string(object_type_name(sequence)));
	return throw_error(cont, str);
}

object* fold(object* args, object* cont) {
	object* syntax_procedure;
	object* trace;
	delist_2(args, &syntax_procedure, &trace);
	
	object* function;
	object* initial;
	object* sequence;
	delist_3(syntax_procedure, &function, &initial, &sequence);
	
	object* proc = sequence_fold_procedure[sequence->type];
	object* call_args = alloc_list_4(function, initial, sequence, trace);
	object* call = alloc_call(proc, call_args, cont);
	
	return perform_call(call);
}

object* eval_and_fold(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* fold_args = alloc_list_1(trace);
	object* fold_call = alloc_call(&fold_proc, fold_args, cont);
	
	object* eval_args = alloc_list_4(syntax, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, alloc_cont(fold_call));
	
	return perform_call(eval_call);
}

void init_higher_order_procedures(void) {
	add_syntax("map", syntax_map, context_value, &eval_and_map);
	add_syntax("filter", syntax_filter, context_value, &eval_and_filter);
	add_syntax("fold", syntax_fold, context_value, &eval_and_fold);
	
	init_primitive(&map, &map_proc);
	init_primitive(&map_undefined, &map_undefined_proc);
	
	init_primitive(&filter, &filter_proc);
	init_primitive(&filter_undefined, &filter_undefined_proc);
	
	init_primitive(&fold, &fold_proc);
	init_primitive(&fold_undefined, &fold_undefined_proc);
	
	for (object_type k = type_none; k < type_count; k++) {
		add_map_procedure(k, &map_undefined_proc);
		add_filter_procedure(k, &filter_undefined_proc);
		add_fold_procedure(k, &fold_undefined_proc);
	}
}
