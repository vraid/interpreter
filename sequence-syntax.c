#include "sequence-syntax.h"

#include "base-syntax.h"
#include "data-structures.h"
#include "global-variables.h"
#include "delist.h"
#include "object-init.h"
#include "call.h"
#include "eval.h"
#include "vectors.h"
#include "streams.h"
#include "sequences.h"
#include "list-util.h"

object map_start_proc;

object* list(object* args, object* cont) {
	object* elements;
	object* environment;
	delist_2(args, &elements, &environment);
	
	object eval_call;
	init_call(&eval_call, &eval_list_elements_proc, args, cont);
	
	return perform_call(&eval_call);
}

object* stream(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object* first;
	object* rest;
	delist_2(syntax, &first, &rest);
	
	object delay;
	init_delay(&delay, rest, environment);
	
	object stream_args[1];
	init_list_1(stream_args, &delay);
	object stream_call;
	init_call(&stream_call, &make_stream_proc, stream_args, cont);
	object stream_cont;
	init_cont(&stream_cont, &stream_call);
	
	object eval_args[2];
	init_list_2(eval_args, first, environment);
	object eval_call;
	init_call(&eval_call, &eval_proc, eval_args, &stream_cont);
	
	return perform_call(&eval_call);
}
	

object* vector(object* args, object* cont) {
	object vector_call;
	init_call(&vector_call, &list_to_vector_proc, empty_list(), cont);
	object vector_cont;
	init_cont(&vector_cont, &vector_call);
	
	object list_call;
	init_call(&list_call, syntax_obj(syntax_list), args, &vector_cont);
	
	return perform_call(&list_call);
}

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
		object next_iter;
		object* rest = next_iterator(&next_iter, unmapped);
		
		object map_args[2];
		init_list_2(map_args, rest, function);
		object map_call;
		init_call(&map_call, &map_single_proc, map_args, cont);
		object map_cont;
		init_cont(&map_cont, &map_call);
		
		object add_args[1];
		init_list_1(add_args, last);
		object add_call;
		init_call(&add_call, &add_to_list_proc, add_args, &map_cont);
		object add_cont;
		init_cont(&add_cont, &add_call);
		
		object function_args[1];
		init_list_1(function_args, sequence_first(unmapped));
		object eval_args[2];
		init_list_2(eval_args, function_args, function);
		object eval_call;
		init_call(&eval_call, &eval_function_call_proc, eval_args, &add_cont);
		
		return perform_call(&eval_call);
	}
}

object* map_start(object* args, object* cont) {
	object* syntax;
	delist_1(args, &syntax);
	
	object* function;
	object* sequence;
	delist_2(syntax, &function, &sequence);
	
	if (is_stream(sequence)) {
		object call;
		init_call(&call, &stream_map_proc, args, cont);
		return perform_call(&call);
	}
	else {
		object convert_call;
		init_call(&convert_call, list_to_sequence_proc(sequence->type), empty_list(), cont);
		object convert_cont;
		init_cont(&convert_cont, &convert_call);
		
		if (is_empty_list(sequence)) {
			return call_cont(&convert_cont, empty_list());
		}
		else {
			object next_iter;
			object* rest = next_iterator(&next_iter, sequence);
			
			object map_args[2];
			init_list_2(map_args, rest, function);
			object list_args[2];
			init_list_2(list_args, &map_single_proc, map_args);
			object list_call;
			init_call(&list_call, &make_list_proc, list_args, &convert_cont);
			object list_cont;
			init_cont(&list_cont, &list_call);
			
			object function_args[1];
			init_list_1(function_args, sequence_first(sequence));
			object eval_args[2];
			init_list_2(eval_args, function_args, function);
			object eval_call;
			init_call(&eval_call, &eval_function_call_proc, eval_args, &list_cont);
			
			return perform_call(&eval_call);
		}
	}
}

object* map(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object start_call;
	init_call(&start_call, &map_start_proc, empty_list(), cont);
	object start_cont;
	init_cont(&start_cont, &start_call);
	
	object eval_args[2];
	init_list_2(eval_args, syntax, environment);
	object eval_call;
	init_call(&eval_call, &eval_list_elements_proc, eval_args, &start_cont);
	
	return perform_call(&eval_call);
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
		object iter;
		object* rest = sequence_rest(&iter, elements);
		
		object fold_args[2];
		init_list_2(fold_args, rest, function);
		object fold_call;
		init_call(&fold_call, &fold_single_proc, fold_args, cont);
		object fold_cont;
		init_cont(&fold_cont, &fold_call);
		
		object function_args[2];
		init_list_2(function_args, value, sequence_first(elements));
		object eval_args[2];
		init_list_2(eval_args, function_args, function);
		object eval_call;
		init_call(&eval_call, &eval_function_call_proc, eval_args, &fold_cont);
		
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
	
	if (is_stream(elements)) {
		object call;
		init_call(&call, &stream_fold_proc, syntax, cont);
		
		return perform_call(&call);
	}
	else {
		object fold_args[3];
		init_list_3(fold_args, initial, elements, function);
		object fold_call;
		init_call(&fold_call, &fold_single_proc, fold_args, cont);
		
		return perform_call(&fold_call);
	}
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
	init_call(&eval_call, &eval_list_elements_proc, eval_args, &fold_cont);
	
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
		init_call(&add_call, &add_to_list_proc, add_args, cont);
		
		return perform_call(&add_call);
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
		object iter;
		object* rest = sequence_rest(&iter, unfiltered);
		
		object filter_args[2];
		init_list_2(filter_args, rest, function);
		object filter_call;
		init_call(&filter_call, &filter_single_proc, filter_args, cont);
		object filter_cont;
		init_cont(&filter_cont, &filter_call);
		
		object* value = sequence_first(unfiltered);
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
		init_call(&eval_call, &eval_function_call_proc, eval_args, &add_cont);
		
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
		if (is_empty_sequence(unfiltered)) {
			return call_cont(cont, empty_list());
		}
		else {
			object* value = sequence_first(unfiltered);
			object iter;
			object* rest = sequence_rest(&iter, unfiltered);
			
			object filter_args[3];
			init_list_3(filter_args, value, rest, function);
			object filter_call;
			init_call(&filter_call, &filter_first_proc, filter_args, cont);
			object filter_cont;
			init_cont(&filter_cont, &filter_call);
			
			object function_args[1];
			init_list_1(function_args, value);
			object eval_args[2];
			init_list_2(eval_args, function_args, function);
			object eval_call;
			init_call(&eval_call, &eval_function_call_proc, eval_args, &filter_cont);
			
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
		init_call(&list_call, &make_list_proc, list_args, cont);
		
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
	
	if (is_stream(elements)) {
		object call;
		init_call(&call, &stream_filter_proc, syntax, cont);
		
		return perform_call(&call);
	}
	else {
		object convert_call;
		init_call(&convert_call, list_to_sequence_proc(elements->type), empty_list(), cont);
		object convert_cont;
		init_cont(&convert_cont, &convert_call);
		
		if (is_empty_sequence(elements)) {
			return call_cont(&convert_cont, empty_list());
		}
		else {
			object* value = sequence_first(elements);
			object iter;
			object* rest = sequence_rest(&iter, elements);
			
			object filter_args[3];
			init_list_3(filter_args, value, rest, function);
			object filter_call;
			init_call(&filter_call, &filter_first_proc, filter_args, &convert_cont);
			object filter_cont;
			init_cont(&filter_cont, &filter_call);
			
			object function_args[1];
			init_list_1(function_args, value);
			object eval_args[2];
			init_list_2(eval_args, function_args, function);
			object eval_call;
			init_call(&eval_call, &eval_function_call_proc, eval_args, &filter_cont);
			
			return perform_call(&eval_call);
		}
	}
}

object* filter(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object start_call;
	init_call(&start_call, &filter_start_proc, empty_list(), cont);
	object start_cont;
	init_cont(&start_cont, &start_call);
	
	object eval_args[2];
	init_list_2(eval_args, syntax, environment);
	object eval_call;
	init_call(&eval_call, &eval_list_elements_proc, eval_args, &start_cont);
	
	return perform_call(&eval_call);
}

void init_sequence_syntax_procedures(void) {
	add_syntax("list", syntax_list, &list);
	add_syntax("stream", syntax_stream, &stream);
	add_syntax("vector", syntax_vector, &vector);
	add_syntax("map", syntax_map, &map);
	add_syntax("fold", syntax_fold, &fold);
	add_syntax("filter", syntax_filter, &filter);
	
	init_primitive(&map_single, &map_single_proc);
	init_primitive(&map_start, &map_start_proc);
	
	init_primitive(&filter_start, &filter_start_proc);
	init_primitive(&filter_first, &filter_first_proc);
	init_primitive(&filter_single, &filter_single_proc);
	init_primitive(&add_or_discard_filtered, &add_or_discard_filtered_proc);
	
	init_primitive(&fold_start, &fold_start_proc);
	init_primitive(&fold_single, &fold_single_proc);
}
