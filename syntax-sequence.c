#include "syntax-sequence.h"

#include <stdlib.h>
#include "syntax-base.h"
#include "data-structures.h"
#include "global-variables.h"
#include "delist.h"
#include "object-init.h"
#include "call.h"
#include "eval.h"
#include "vectors.h"
#include "streams.h"
#include "higher-order.h"

object* list(object* args, object* cont) {
	object* elements;
	object* environment;
	object* trace;
	delist_3(args, &elements, &environment, &trace);
	
	object* eval_args = alloc_list_4(elements, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_list_elements_proc, eval_args, cont);
	
	return perform_call(eval_call);
}

object* stream(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* first;
	object* rest;
	delist_2(syntax, &first, &rest);
	
	object* delay_args = alloc_list_3(rest, environment, trace);
	object* delay = alloc_delay(delay_args);
	
	object* stream_args = alloc_list_1(delay);
	object* stream_call = alloc_call(&make_stream_proc, stream_args, cont);
	
	object* eval_args = alloc_list_4(first, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(stream_call));
	
	return perform_call(eval_call);
}

object* vector(object* args, object* cont) {
	object* vector_call = alloc_call(&list_to_vector_proc, empty_list(), cont);	
	object* list_call = alloc_call(syntax_procedure_obj(syntax_list), args, alloc_cont(vector_call));
	
	return perform_call(list_call);
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

void init_sequence_syntax_procedures(void) {
	add_syntax("list", syntax_list, context_value, &list);
	add_syntax("stream", syntax_stream, context_value, &stream);
	add_syntax("vector", syntax_vector, context_value, &vector);
	add_syntax("map", syntax_map, context_value, &eval_and_map);
	add_syntax("fold", syntax_fold, context_value, &eval_and_fold);
	add_syntax("filter", syntax_filter, context_value, &eval_and_filter);
}
