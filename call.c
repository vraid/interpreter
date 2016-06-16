#include "call.h"
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
	save_call(call);
	setjmp(jump_buffer);
	char p;
	stack_top = &p;
	return perform_call(saved_call);	
}

char stack_full(void) {
	char p;
	return (stack_top > (&p + max_stack_data));
}

object* perform_call(object* call) {
	if ((call_count >= max_call_count) || stack_full() || max_mutations_reached()) {
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
	else if (is_syntax(function)) {
		return (*(function->data.syntax.proc))(call_arguments(call), call_continuation(call));
	}		
	else {
		fprintf(stderr, "faulty call\n");
		return no_object();
	}
}

object* call_cont(object* cont, object* arg) {
	if (is_discarding_continuation(cont)) {
		fprintf(stderr, "discarding continuation called with argument\n");
		return no_object();
	}
	else {
		object* call = continuation_call(cont);
		object ls;
		init_list_cell(&ls, arg, call_arguments(call));
		object new_call;
		// copying the modified call to the stack simplifies the gc
		init_call(&new_call, call_function(call), &ls, call_continuation(call));
		return perform_call(&new_call);
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
