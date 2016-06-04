#include "repl-top.h"
#include "data-structures.h"
#include "global-variables.h"
#include "object-init.h"
#include "read.h"
#include "eval.h"
#include "print.h"
#include "delist.h"
#include "call.h"

object repl_eval_entry_proc;
object repl_print_or_read_proc;
object repl_print_entry_proc;

object placeholder_eval_proc;

object* placeholder_eval(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	printf("eval\n");
	
	if (is_internal_error(value)) {
		return call_cont(cont, value);
	}
	else {
		return call_cont(cont, value);
	}
}

object* repl_read_entry(object* args, object* cont) {
	object* environment;
	delist_1(args, &environment);
	
	object eval_call;
	init_call(&eval_call, &repl_eval_entry_proc, args, cont);
	object next_cont;
	init_cont(&next_cont, &eval_call);
	
	object input_port;
	init_object(location_stack, type_file_port, &input_port);
	input_port.data.file_port.file = stdin;
	
	object ls2[1];
	init_list_1(ls2, &input_port);
	
	object call;
	init_call(&call, &read_value_proc, ls2, &next_cont);
	
	return perform_call(&call);
}

object* repl_eval_entry(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object ls[1];
	init_list_1(ls, environment);
	
	object print_call;
	init_call(&print_call, &repl_print_or_read_proc, ls, cont);
	object next_cont;
	init_cont(&next_cont, &print_call);
	object call;
	init_call(&call, &placeholder_eval_proc, args, &next_cont);
	
	return perform_call(&call);
}

object* repl_print_or_read(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object call;
	
	if (is_internal_error(value)) {
		printf("error: %s\n", string_value(internal_error_message(value)));
		init_call(&call, &repl_read_entry_proc, args, cont);
	}
	if (is_environment(value)) {
		object ls[1];
		init_list_1(ls, environment);
		init_call(&call, &repl_read_entry_proc, ls, cont);
	}
	else {
		init_call(&call, &repl_print_entry_proc, args, cont);
	}
	
	return perform_call(&call);
}

object* repl_print_entry(object* args, object* cont) {
	object* value;
	object* environment;
	delist_2(args, &value, &environment);
	
	object ls[1];
	init_list_1(ls, environment);
	
	object read_call;
	init_call(&read_call, &repl_read_entry_proc, ls, cont);
	object next_cont;
	init_discarding_cont(&next_cont, &read_call);
	
	object newline_call;
	init_call(&newline_call, &print_newline_proc, empty_list(), &next_cont);
	object newline_cont;
	init_discarding_cont(&newline_cont, &newline_call);
	
	object ls2[1];
	init_list_1(ls2, value);
	object call;
	init_call(&call, &print_proc, ls2, &newline_cont);
	
	return perform_call(&call);
}

void init_repl_procedures(void) {
	init_primitive_procedure(&repl_read_entry_proc, &repl_read_entry);
	init_primitive_procedure(&repl_eval_entry_proc, &repl_eval_entry);
	init_primitive_procedure(&repl_print_or_read_proc, &repl_print_or_read);
	init_primitive_procedure(&repl_print_entry_proc, &repl_print_entry);
	init_primitive_procedure(&placeholder_eval_proc, &placeholder_eval);
}
