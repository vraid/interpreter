#include "higher-order.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "delist.h"
#include "list-util.h"
#include "call.h"
#include "eval.h"
#include "sequences.h"
#include "vectors.h"
#include "streams.h"

object map_single_proc;

object* map_single(object* args, object* cont) {
	object* last;
	object* unmapped;
	object* function;
	delist_3(args, &last, &unmapped, &function);
	
	if (is_empty_sequence(unmapped)) {
		return call_discarding_cont(cont);
	}
	else {
		object* rest = alloc_next_iterator(unmapped);
		
		object* map_args = alloc_list_2(rest, function);
		object* map_call = alloc_call(&map_single_proc, map_args, cont);
		object* map_cont = alloc_cont(map_call);
		
		object* add_args = alloc_list_1(last);
		object* add_call = alloc_call(&add_to_list_proc, add_args, map_cont);
		object* add_cont = alloc_cont(add_call);
		
		object* function_args = alloc_list_1(sequence_first(unmapped));
		object* eval_args = alloc_list_2(function_args, function);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, add_cont);
		
		return perform_call(eval_call);
	}
}

object* map(object* args, object* cont) {
	object* syntax_procedure;
	delist_1(args, &syntax_procedure);
	
	object* function;
	object* sequence;
	delist_2(syntax_procedure, &function, &sequence);
	
	if (is_stream(sequence)) {
		object* call = alloc_call(&stream_map_proc, args, cont);
		return perform_call(call);
	}
	else {
		object* convert_call = alloc_call(list_to_sequence_proc(sequence->type), empty_list(), cont);
		object* convert_cont = alloc_cont(convert_call);
		
		if (is_empty_list(sequence)) {
			return call_cont(convert_cont, empty_list());
		}
		else {
			object* rest = alloc_next_iterator(sequence);
			
			object* map_args = alloc_list_2(rest, function);
			object* list_args = alloc_list_2(&map_single_proc, map_args);
			object* list_call = alloc_call(&make_list_proc, list_args, convert_cont);
			object* list_cont = alloc_cont(list_call);
			
			object* function_args = alloc_list_1(sequence_first(sequence));
			object* eval_args = alloc_list_2(function_args, function);
			object* eval_call = alloc_call(&eval_function_call_proc, eval_args, list_cont);
			
			return perform_call(eval_call);
		}
	}
}

object fold_single_proc;

object* fold_single(object* args, object* cont) {
	object* value;
	object* elements;
	object* function;
	delist_3(args, &value, &elements, &function);
	
	if (is_empty_sequence(elements)) {
		return call_cont(cont, value);
	}
	else {
		object* rest = alloc_sequence_rest(elements);
		
		object* fold_args = alloc_list_2(rest, function);
		object* fold_call = alloc_call(&fold_single_proc, fold_args, cont);
		object* fold_cont = alloc_cont(fold_call);
		
		object* function_args = alloc_list_2(value, sequence_first(elements));
		object* eval_args = alloc_list_2(function_args, function);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, fold_cont);
		
		return perform_call(eval_call);
	}
}

object* fold(object* args, object* cont) {
	object* syntax_procedure;
	delist_1(args, &syntax_procedure);
	
	object* function;
	object* initial;
	object* elements;
	delist_3(syntax_procedure, &function, &initial, &elements);
	
	if (is_stream(elements)) {
		object* call = alloc_call(&stream_fold_proc, syntax_procedure, cont);
		
		return perform_call(call);
	}
	else {
		object* fold_args = alloc_list_3(initial, elements, function);
		object* fold_call = alloc_call(&fold_single_proc, fold_args, cont);
		
		return perform_call(fold_call);
	}
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
		object* add_args = alloc_list_2(value, last);
		object* add_call = alloc_call(&add_to_list_proc, add_args, cont);
		
		return perform_call(add_call);
	}
}

object filter_single_proc;

object* filter_single(object* args, object* cont) {
	object* last;
	object* unfiltered;
	object* function;
	delist_3(args, &last, &unfiltered, &function);
	
	if (is_empty_sequence(unfiltered)) {
		return call_discarding_cont(cont);
	}
	else {
		object* rest = alloc_sequence_rest(unfiltered);
		
		object* filter_args = alloc_list_2(rest, function);
		object* filter_call = alloc_call(&filter_single_proc, filter_args, cont);
		object* filter_cont = alloc_cont(filter_call);
		
		object* value = sequence_first(unfiltered);
		object* add_args = alloc_list_2(value, last);
		object* add_call = alloc_call(&add_or_discard_filtered_proc, add_args, filter_cont);
		object* add_cont = alloc_cont(add_call);
		
		object* function_args = alloc_list_1(value);
		object* eval_args = alloc_list_2(function_args, function);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, add_cont);
		
		return perform_call(eval_call);
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
		if (is_empty_sequence(unfiltered)) {
			return call_cont(cont, empty_list());
		}
		else {
			object* value = sequence_first(unfiltered);
			object* rest = alloc_sequence_rest(unfiltered);
			
			object* filter_args = alloc_list_3(value, rest, function);
			object* filter_call = alloc_call(&filter_first_proc, filter_args, cont);
			object* filter_cont = alloc_cont(filter_call);
			
			object* function_args = alloc_list_1(value);
			object* eval_args = alloc_list_2(function_args, function);
			object* eval_call = alloc_call(&eval_function_call_proc, eval_args, filter_cont);
			
			return perform_call(eval_call);
		}
	}
	// start making a list
	else {
		object* filter_args = alloc_list_2(unfiltered, function);
		object* list_args = alloc_list_3(value, &filter_single_proc, filter_args);
		object* list_call = alloc_call(&make_list_proc, list_args, cont);
		
		return perform_call(list_call);
	}
}

object* filter(object* args, object* cont) {
	object* syntax_procedure;
	delist_1(args, &syntax_procedure);
	
	object* function;
	object* elements;
	delist_2(syntax_procedure, &function, &elements);
	
	if (is_stream(elements)) {
		object* call = alloc_call(&stream_filter_proc, syntax_procedure, cont);
		
		return perform_call(call);
	}
	else {
		object* convert_call = alloc_call(list_to_sequence_proc(elements->type), empty_list(), cont);
		object* convert_cont = alloc_cont(convert_call);
		
		if (is_empty_sequence(elements)) {
			return call_cont(convert_cont, empty_list());
		}
		else {
			object* value = sequence_first(elements);
			object* rest = alloc_sequence_rest(elements);
			
			object* filter_args = alloc_list_3(value, rest, function);
			object* filter_call = alloc_call(&filter_first_proc, filter_args, convert_cont);
			object* filter_cont = alloc_cont(filter_call);
			
			object* function_args = alloc_list_1(value);
			object* eval_args = alloc_list_2(function_args, function);
			object* eval_call = alloc_call(&eval_function_call_proc, eval_args, filter_cont);
			
			return perform_call(eval_call);
		}
	}
}

void init_higher_order_procedures(void) {
	init_primitive(&map_single, &map_single_proc);
	init_primitive(&map, &map_proc);
	
	init_primitive(&filter, &filter_proc);
	init_primitive(&filter_first, &filter_first_proc);
	init_primitive(&filter_single, &filter_single_proc);
	init_primitive(&add_or_discard_filtered, &add_or_discard_filtered_proc);
	
	init_primitive(&fold, &fold_proc);
	init_primitive(&fold_single, &fold_single_proc);
}
