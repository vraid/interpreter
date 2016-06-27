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
	
	object stream;
	init_stream(&stream, first, rest);
	
	return call_cont(cont, &stream);
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
		object rest_list[2];
		init_list_2(rest_list, &eval_stream_rest_proc, stream);
		object map_list[3];
		init_list_3(map_list, &syntax_procedure[syntax_map], function, rest_list);
		object delay;
		init_delay(&delay, map_list, empty_environment());
		
		object make_args[1];
		init_list_1(make_args, &delay);
		object make_call;
		init_call(&make_call, &make_stream_proc, make_args, cont);
		object make_cont;
		init_cont(&make_cont, &make_call);
		
		object function_args[1];
		init_list_1(function_args, stream_first(stream));
		object eval_args[2];
		init_list_2(eval_args, function_args, function);
		object eval_call;
		init_call(&eval_call, eval_function_call_proc(), eval_args, &make_cont);
		
		return perform_call(&eval_call);
	}
}

object* throw_stream_rest_error(object* cont, object* rest) {
	fprintf(stderr, "invalid stream rest type: %s\n", type_name[rest->type]);
	return throw_error(cont, "invalid stream rest type");
}

object* eval_stream_rest(object* args, object* cont) {
	object* stream;
	delist_1(args, &stream);
	
	if (is_empty_stream(stream)) {
		return throw_error(cont, "rest on empty stream");
	}
	
	object* rest = stream_rest(stream);
	
	if (delay_evaluated(rest)) {
		return call_cont(cont, delay_value(rest));
	}
	else {		
		object eval_args[1];
		init_list_1(eval_args, rest);
		object eval_call;
		init_call(&eval_call, &eval_force_proc, eval_args, cont);
		
		return perform_call(&eval_call);
	}
}

void init_stream_procedures(void) {
	init_primitive_procedure(&make_stream_proc, &make_stream);
	init_primitive_procedure(&stream_map_proc, &stream_map);
	init_primitive_procedure(&eval_stream_rest_proc, &eval_stream_rest);
}
