#include "streams.h"

#include <stdlib.h>
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "list-util.h"
#include "string-util.h"
#include "call.h"
#include "delist.h"
#include "eval.h"
#include "lists.h"
#include "sequences.h"
#include "higher-order.h"
#include "syntax-base.h"

object* stream(object* args, object* cont) {
	object* syntax;
	object* environment;
	object* trace;
	delist_3(args, &syntax, &environment, &trace);
	
	object* first;
	object* rest;
	delist_2(syntax, &first, &rest);
	
	object* delay_args = alloc_list_4(rest, environment, default_context(), trace);
	object* delay = alloc_delay(delay_args);
	
	object* stream_args = alloc_list_1(delay);
	object* stream_call = alloc_call(&make_stream_proc, stream_args, cont);
	
	object* eval_args = alloc_list_4(first, environment, default_context(), trace);
	object* eval_call = alloc_call(&eval_proc, eval_args, alloc_cont(stream_call));
	
	return perform_call(eval_call);
}

object* make_stream(object* args, object* cont) {
	object* first;
	object* rest;
	delist_2(args, &first, &rest);
	
	object* stream = alloc_stream(first, rest);
	
	return call_cont(cont, stream);
}

object eval_stream_rest_proc;

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

object stream_rest_proc;

object* stream_seq_rest(object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	
	object* call = alloc_call(&eval_stream_rest_proc, alloc_list_1(stream), cont);
	return perform_call(call);
}

object stream_to_list_accumulate_proc;

object* stream_to_list_accumulate(object* args, object* cont) {
	object* stream;
	object* ls;
	delist_2(args, &stream, &ls);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, ls);
	}
	else {
		object* first = stream_first(stream);		
		ls = alloc_list_cell(first, ls);
		
		object* next_call = alloc_call(&stream_to_list_accumulate_proc, alloc_list_1(ls), cont);
		object* eval_call = alloc_call(&eval_stream_rest_proc, alloc_list_1(stream), alloc_cont(next_call));
		
		return perform_call(eval_call);
	}
}

object stream_to_list_proc;

object* stream_to_list(object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, empty_list());
	}
	else {
		object* ls = alloc_list_1(stream_first(stream));
		
		object* reverse_call = alloc_call(&reverse_list_proc, empty_list(), cont);
		object* next_call = alloc_call(&stream_to_list_accumulate_proc, alloc_list_1(ls), alloc_cont(reverse_call));
		object* eval_call = alloc_call(&eval_stream_rest_proc, alloc_list_1(stream), alloc_cont(next_call));
		
		return perform_call(eval_call);
	}
}

object* stream_map(object* args, object* cont) {
	object* function;
	object* stream;
	object* trace;
	delist_3(args, &function, &stream, &trace);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, empty_stream());
	}
	else {
		object* rest_list = alloc_list_2(&eval_stream_rest_proc, stream);
		object* map_list = alloc_list_3(syntax_procedure_obj(syntax_map), function, rest_list);
		object* delay_args = alloc_list_4(map_list, empty_list(), default_context(), trace);
		object* delay = alloc_delay(delay_args);
		
		object* make_args = alloc_list_1(delay);
		object* make_call = alloc_call(&make_stream_proc, make_args, cont);
		
		object* function_args = alloc_list_1(stream_first(stream));
		object* eval_args = alloc_list_4(function_args, function, default_context(), trace);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, alloc_cont(make_call));
		
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
	object* trace;
	delist_5(args, &result, &value, &stream, &function, &trace);
	
	if (is_false(result)) {
		object* rest_args = alloc_list_3(stream, function, trace);
		object* rest_call = alloc_call(&stream_filter_rest_eval_proc, rest_args, cont);
		
		return perform_call(rest_call);
	}
	else {
		object* rest_list = alloc_list_2(&eval_stream_rest_proc, stream);
		object* filter_list = alloc_list_3(syntax_procedure_obj(syntax_filter), function, rest_list);
		object* delay_args = alloc_list_4(filter_list, empty_list(), default_context(), trace);
		object* delay = alloc_delay(delay_args);

		object* stream = alloc_stream(value, delay);
		
		return call_cont(cont, stream);
	}
}

object stream_filter_func_eval_proc;

object* stream_filter_func_eval(object* args, object* cont) {
	object* stream;
	object* function;
	object* trace;
	delist_3(args, &stream, &function, &trace);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, empty_stream());
	}
	else {
		object* first = stream_first(stream);
		
		object* stream_args = alloc_list_4(first, stream, function, trace);
		object* stream_call = alloc_call(&stream_filter_build_proc, stream_args, cont);

		object* function_args = alloc_list_1(first);
		object* eval_args = alloc_list_4(function_args, function, default_context(), trace);
		object* eval_call = alloc_call(&eval_function_call_proc, eval_args, alloc_cont(stream_call));
		
		return perform_call(eval_call);
	}
}

object* stream_filter_rest_eval(object* args, object* cont) {
	object* stream;
	object* function;
	object* trace;
	delist_3(args, &stream, &function, &trace);
	
	object* eval_args = alloc_list_2(function, trace);
	object* eval_call = alloc_call(&stream_filter_func_eval_proc, eval_args, cont);
	
	object* rest_args = alloc_list_1(stream);
	object* rest_call = alloc_call(&eval_stream_rest_proc, rest_args, alloc_cont(eval_call));
	
	return perform_call(rest_call);
}

object* stream_filter(object* args, object* cont) {
	object* function;
	object* stream;
	object* trace;
	delist_3(args, &function, &stream, &trace);
	
	if (is_empty_stream(stream)) {
		return call_cont(cont, empty_stream());
	}
	else {
		object* eval_args = alloc_list_3(stream, function, trace);
		object* eval_call = alloc_call(&stream_filter_func_eval_proc, eval_args, cont);
		
		return perform_call(eval_call);
	}
}

object stream_fold_proc;

object* stream_fold(object* args, object* cont) {
	object* function;
	object* initial;
	object* stream;
	object* trace;
	delist_4(args, &function, &initial, &stream, &trace);
	
	object* next_args = alloc_list_3(function, initial, trace);
	object* next_call = alloc_call(&fold_as_list_proc, next_args, cont);
	
	object* list_args = alloc_list_1(stream);
	object* list_call = alloc_call(&stream_to_list_proc, list_args, alloc_cont(next_call));
	
	return perform_call(list_call);
}

object* throw_stream_rest_error(object* cont, object* rest) {
	object* str = alloc_string_append_2(alloc_string("invalid stream rest type: "), alloc_string(object_type_name(rest)));
	return throw_error(cont, str);
}

void init_stream_procedures(void) {
	add_syntax("stream", syntax_stream, context_value, &stream);
	init_primitive(&make_stream, &make_stream_proc);
	
	init_primitive(&stream_to_list, &stream_to_list_proc);
	init_primitive(&stream_to_list_accumulate, &stream_to_list_accumulate_proc);
	
	add_first_procedure(type_stream, &stream_first);
	init_primitive(&stream_seq_rest, &stream_rest_proc);
	add_rest_procedure(type_stream, &stream_rest_proc);
	
	init_primitive(&stream_map, &stream_map_proc);
	add_map_procedure(type_stream, &stream_map_proc);
	
	init_primitive(&stream_filter, &stream_filter_proc);
	init_primitive(&stream_filter_build, &stream_filter_build_proc);
	init_primitive(&stream_filter_func_eval, &stream_filter_func_eval_proc);
	init_primitive(&stream_filter_rest_eval, &stream_filter_rest_eval_proc);
	add_filter_procedure(type_stream, &stream_filter_proc);
	
	init_primitive(&stream_fold, &stream_fold_proc);
	add_fold_procedure(type_stream, &stream_fold_proc);
	
	init_primitive(&eval_stream_rest, &eval_stream_rest_proc);
}
