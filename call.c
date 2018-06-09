#include "call.h"

#include <stdlib.h>
#include <setjmp.h>
#include "data-structures.h"
#include "object-init.h"
#include "global-variables.h"
#include "memory-handling.h"

object* saved_call;
char* stack_top;
jmp_buf jump_buffer;
int call_count = 0;
#define max_call_count 1024*1024

void save_call(object* call) {
	saved_call = call;
}

object* top_call(object* call) {
	char p;
	stack_top = &p;
	save_call(call);
	printf("stack top is at %p\n", (void*)&p);
	setjmp(jump_buffer);
	return perform_call(saved_call);	
}

char stack_full(void) {
	char p;
	return (stack_top > (&p + max_stack_data));
}

object* perform_call(object* call) {
	if ((call_count >= max_call_count) || stack_full() || max_stack_references_reached()) {
		call_count = 0;
		save_call(call);
		perform_gc(&saved_call);
		longjmp(jump_buffer, 0);
	}
	call_count++;
	object* function = call_function(call);
	if (is_function(function)) {
		return call;
	}
	else if (is_primitive_procedure(function)) {
		return (*(function->data.primitive_procedure.proc))(call_arguments(call), call_continuation(call));
	}
	else if (is_syntax_procedure(function)) {
		return (*(function->data.syntax_procedure.proc))(call_arguments(call), call_continuation(call));
	}		
	else {
		fprintf(stderr, "faulty call\n");
		return no_object();
	}
}

object* throw_trace_error_string(object* cont, object* origin, char* str);

object* throw_error(object* cont, object* message) {
	return throw_trace_error(cont, false(), message);
}

object* throw_error_string(object* cont, char* str) {
	return throw_trace_error_string(cont, false(), str);
}

object* throw_trace_error(object* cont, object* trace, object* message) {
	object* e = alloc_internal_error(trace, message);
	
	return call_cont(cont, e);
}

object* throw_trace_error_string(object* cont, object* trace, char* str) {
	object* obj = alloc_string(str);
	
	return throw_trace_error(cont, trace, obj);
}

object* call_cont(object* cont, object* arg) {
	object* call = continuation_call(cont);
	// bypasses calls and passes errors along continuations until the next catching continuation
	if (is_internal_error(arg) && !is_catching_continuation(cont)) {
		return call_cont(call_continuation(call), arg);
	}
	else if (is_discarding_continuation(cont)) {
		fprintf(stderr, "discarding continuation called with argument\n");
		return no_object();
	}
	else {
		object* ls = alloc_list_cell(arg, call_arguments(call));
		// copying the modified call to the stack simplifies the gc
		object* new_call = alloc_call(call_function(call), ls, call_continuation(call));
		return perform_call(new_call);
	}
}

object* call_discarding_cont(object* cont) {
	if (!is_discarding_continuation(cont)) {
		fprintf(stderr, "normal continuation called without argument\n");
		return no_object();
	}
	else {
		return perform_call(continuation_call(cont));
	}
}
