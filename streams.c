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
	init_primitive_procedure(&eval_stream_rest_proc, &eval_stream_rest);
}
