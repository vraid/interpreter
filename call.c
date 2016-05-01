#include "call.h"
#include "data-structures.h"
#include "object-init.h"
#include "global-variables.h"

object* saved_call;

void save_call(object* call) {
	saved_call = call;
}

object* top_call(object* call) {
	save_call(call);
	// if setjmp
	return perform_call(saved_call);	
}

object* perform_call(object* call) {
	// if stack full
	// save call, jump back
	object* function = call_function(call);
	if (is_function(function)) {
		return call;
	}
	else if (is_primitive_procedure(function)) {
		return (*(function->data.primitive_procedure.proc))(call_arguments(call), call_continuation(call));
	}
	else {
		printf("faulty call\n");
		return no_object();
	}
}

object* call_cont(object* cont, object* arg) {
	object* call = continuation_call(cont);
	object ls;
	init_list_cell(&ls, arg, call_arguments(call));
	call->data.call.arguments = &ls;
	
	return perform_call(call);
}
