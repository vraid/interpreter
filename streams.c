#include "streams.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "call.h"
#include "delist.h"
#include "memory-handling.h"
#include "eval.h"
#include "base-syntax.h"

object* make_stream(object* args, object* cont) {
	object* first;
	object* rest;
	delist_2(args, &first, &rest);
	
	object* stream = alloc_stream(first, rest);
	
	return call_cont(cont, stream);
}

object* stream_map(object* args, object* cont) {
	object* syntax;
	delist_1(args, &syntax);
	
	object* function;
	object* stream;
	delist_2(syntax, &function, &stream);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, empty_stream());
	}
	else {
		object* rest_list = alloc_list_2(&eval_stream_rest_proc, stream);
		object* map_list = alloc_list_3(syntax_procedure_obj(syntax_map), function, rest_list);
		object* delay_args = alloc_list_2(map_list, empty_environment());
		object* delay = alloc_delay(delay_args);
		
		object* make_args = alloc_list_1(delay);
		object* make_call = alloc_call(&make_stream_proc, make_args, cont);
		object* make_cont = alloc_cont(make_call);
		
		object* function_args = alloc_list_1(stream_first(stream));
		object* eval_args = alloc_list_2(function_args, function);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, make_cont);
		
		return perform_call(eval_call);
	}
}

object stream_fold_three_proc;

object* stream_fold_three(object* args, object* cont) {
	object* stream;
	object* function;
	object* value;
	delist_3(args, &stream, &function, &value);
	
	object* call_args = alloc_list_3(function, value, stream);
	object* call = alloc_call(&stream_fold_proc, call_args, cont);
	
	return perform_call(call);
}

object stream_fold_two_proc;

object* stream_fold_two(object* args, object* cont) {
	object* value;
	object* function;
	object* stream;
	delist_3(args, &value, &function, &stream);
	
	object* next_args = alloc_list_2(function, value);
	object* next_call = alloc_call(&stream_fold_three_proc, next_args, cont);
	object* next_cont = alloc_cont(next_call);
		
	object* rest_args = alloc_list_1(stream);
	object* rest_call = alloc_call(&eval_stream_rest_proc, rest_args, next_cont);
	
	return perform_call(rest_call);
}

object* stream_fold(object* args, object* cont) {
	object* function;
	object* initial;
	object* stream;
	delist_3(args, &function, &initial, &stream);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, initial);
	}
	else {
		object* next_args = alloc_list_2(function, stream);
		object* next_call = alloc_call(&stream_fold_two_proc, next_args, cont);
		object* next_cont = alloc_cont(next_call);
		
		object* function_args = alloc_list_2(initial, stream_first(stream));
		object* eval_list = alloc_list_2(function_args, function);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_list, next_cont);
		
		return perform_call(eval_call);	
	}
}

object stream_filter_rest_eval_proc;
object stream_filter_build_proc;

object* stream_filter_build(object* args, object* cont) {
	object* result;
	object* value;
	object* stream;
	object* function;
	delist_4(args, &result, &value, &stream, &function);
	
	if (is_false(result)) {
		object* rest_args = alloc_list_2(stream, function);
		object* rest_call = alloc_call(&stream_filter_rest_eval_proc, rest_args, cont);
		
		return perform_call(rest_call);
	}
	else {
		object* rest_list = alloc_list_2(&eval_stream_rest_proc, stream);
		object* filter_list = alloc_list_3(syntax_procedure_obj(syntax_filter), function, rest_list);
		object* delay_args = alloc_list_2(filter_list, empty_environment());
		object* delay = alloc_delay(delay_args);

		object* stream = alloc_stream(value, delay);
		
		return call_cont(cont, stream);
	}
}

object stream_filter_func_eval_proc;

object* stream_filter_func_eval(object* args, object* cont) {
	object* stream;
	object* function;
	delist_2(args, &stream, &function);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, empty_stream());
	}
	else {
		object* first = stream_first(stream);
		
		object* stream_args = alloc_list_3(first, stream, function);
		object* stream_call = alloc_call(&stream_filter_build_proc, stream_args, cont);
		object* stream_cont = alloc_cont(stream_call);

		object* function_args = alloc_list_1(first);
		object* eval_args = alloc_list_2(function_args, function);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, stream_cont);
		
		return perform_call(eval_call);
	}
}

object* stream_filter_rest_eval(object* args, object* cont) {
	object* stream;
	object* function;
	delist_2(args, &stream, &function);
	
	object* eval_args = alloc_list_1(function);
	object* eval_call = alloc_call(&stream_filter_func_eval_proc, eval_args, cont);
	object* eval_cont = alloc_cont(eval_call);
	
	object* rest_args = alloc_list_1(stream);
	object* rest_call = alloc_call(&eval_stream_rest_proc, rest_args, eval_cont);
	
	return perform_call(rest_call);
}

object* stream_filter(object* args, object* cont) {
	object* function;
	object* stream;
	delist_2(args, &function, &stream);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, empty_stream());
	}
	else {
		object* eval_args = alloc_list_2(stream, function);
		object* eval_call = alloc_call(&stream_filter_func_eval_proc, eval_args, cont);
		
		return perform_call(eval_call);
	}
}

object* throw_stream_rest_error(object* cont, object* rest) {
	fprintf(stderr, "invalid stream rest type: %s\n", object_type_name(rest));
	return throw_error_string(cont, "invalid stream rest type");
}

object* eval_stream_rest(object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	
	if (is_empty_stream(stream)) {
		return throw_error_string(cont, "rest on empty stream");
	}
	
	object* rest = stream_rest(stream);
	
	if (delay_evaluated(rest)) {
		return call_cont(cont, delay_value(rest));
	}
	else {		
		object* eval_args = alloc_list_1(rest);
		object* eval_call = alloc_call(&eval_force_proc, eval_args, cont);
		
		return perform_call(eval_call);
	}
}

void init_stream_procedures(void) {
	init_primitive(&make_stream, &make_stream_proc);
	init_primitive(&stream_map, &stream_map_proc);
	init_primitive(&stream_fold, &stream_fold_proc);
	init_primitive(&stream_fold_two, &stream_fold_two_proc);
	init_primitive(&stream_fold_three, &stream_fold_three_proc);
	init_primitive(&stream_filter, &stream_filter_proc);
	init_primitive(&stream_filter_build, &stream_filter_build_proc);
	init_primitive(&stream_filter_func_eval, &stream_filter_func_eval_proc);
	init_primitive(&stream_filter_rest_eval, &stream_filter_rest_eval_proc);
	init_primitive(&eval_stream_rest, &eval_stream_rest_proc);
}
