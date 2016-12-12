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
#include "higher-order.h"

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

object* eval_and_map(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object start_call;
	init_call(&start_call, &map_proc, empty_list(), cont);
	object start_cont;
	init_cont(&start_cont, &start_call);
	
	object eval_args[2];
	init_list_2(eval_args, syntax, environment);
	object eval_call;
	init_call(&eval_call, &eval_list_elements_proc, eval_args, &start_cont);
	
	return perform_call(&eval_call);
}

object* eval_and_fold(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object fold_call;
	init_call(&fold_call, &fold_proc, empty_list(), cont);
	object fold_cont;
	init_cont(&fold_cont, &fold_call);
	
	object eval_args[2];
	init_list_2(eval_args, syntax, environment);
	object eval_call;
	init_call(&eval_call, &eval_list_elements_proc, eval_args, &fold_cont);
	
	return perform_call(&eval_call);
}

object* eval_and_filter(object* args, object* cont) {
	object* syntax;
	object* environment;
	delist_2(args, &syntax, &environment);
	
	object start_call;
	init_call(&start_call, &filter_proc, empty_list(), cont);
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
	add_syntax("map", syntax_map, &eval_and_map);
	add_syntax("fold", syntax_fold, &eval_and_fold);
	add_syntax("filter", syntax_filter, &eval_and_filter);
}
